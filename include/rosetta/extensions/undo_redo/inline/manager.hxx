namespace rosetta::core {

    class PropertyChangeCommand {
    public:
        std::string                      field_name;
        Any                              old_value;
        Any                              new_value;
        std::function<void(const Any &)> apply_func;

        void undo() {
            if (apply_func) {
                apply_func(old_value);
            }
        }

        void redo() {
            if (apply_func) {
                apply_func(new_value);
            }
        }
    };

    template <typename T, typename ValueType>
    inline void UndoRedoManager::applyChange(T *object, const std::string &field_name,
                                             const ValueType   &new_value,
                                             const std::string &description) {
        const auto &meta = Registry::instance().get<T>();

        // Capture old value
        Any old_val = meta.get_field(*object, field_name);

        // Apply the change
        meta.set_field(*object, field_name, Any(new_value));

        // Create command
        PropertyChangeCommand cmd;
        cmd.field_name = field_name;
        cmd.old_value  = old_val;
        cmd.new_value  = Any(new_value);
        cmd.apply_func = [object, field_name, &meta](const Any &value) {
            meta.set_field(*object, field_name, value);
        };

        // Store description if provided
        if (!description.empty()) {
            descriptions_.resize(undo_stack_.size() + 1);
            descriptions_.back() = description;
        }

        // Push to undo stack
        undo_stack_.push_back(std::move(cmd));

        // Clear redo stack
        redo_stack_.clear();
        redo_descriptions_.clear();
    }

    inline void UndoRedoManager::undo() {
        if (undo_stack_.empty()) {
            std::cout << "Nothing to undo\n";
            return;
        }

        auto command = std::move(undo_stack_.back());
        undo_stack_.pop_back();

        command.undo();

        redo_stack_.push_back(std::move(command));

        // Move description
        if (!descriptions_.empty()) {
            redo_descriptions_.push_back(descriptions_.back());
            descriptions_.pop_back();
        }

        std::cout << "[UNDO]";
        if (!redo_descriptions_.empty() && !redo_descriptions_.back().empty()) {
            std::cout << " " << redo_descriptions_.back();
        }
        std::cout << "\n";
    }

    inline void UndoRedoManager::redo() {
        if (redo_stack_.empty()) {
            std::cout << "Nothing to redo\n";
            return;
        }

        auto command = std::move(redo_stack_.back());
        redo_stack_.pop_back();

        command.redo();

        undo_stack_.push_back(std::move(command));

        // Move description
        if (!redo_descriptions_.empty()) {
            descriptions_.push_back(redo_descriptions_.back());
            redo_descriptions_.pop_back();
        }

        std::cout << "[REDO]";
        if (!descriptions_.empty() && !descriptions_.back().empty()) {
            std::cout << " " << descriptions_.back();
        }
        std::cout << "\n";
    }

    inline bool UndoRedoManager::canUndo() const {
        return !undo_stack_.empty();
    }

    inline bool UndoRedoManager::canRedo() const {
        return !redo_stack_.empty();
    }

    inline void UndoRedoManager::clear() {
        undo_stack_.clear();
        redo_stack_.clear();
        descriptions_.clear();
        redo_descriptions_.clear();
    }

    inline size_t UndoRedoManager::undoCount() const {
        return undo_stack_.size();
    }

    inline size_t UndoRedoManager::redoCount() const {
        return redo_stack_.size();
    }

    inline void UndoRedoManager::printHistory() const {
        std::cout << "\n=== Undo/Redo History ===\n";
        std::cout << "Undo stack size: " << undo_stack_.size() << "\n";
        std::cout << "Redo stack size: " << redo_stack_.size() << "\n";

        if (!descriptions_.empty()) {
            std::cout << "Undo operations:\n";
            for (size_t i = 0; i < descriptions_.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << descriptions_[i] << "\n";
            }
        }

        if (!redo_descriptions_.empty()) {
            std::cout << "Redo operations:\n";
            for (size_t i = 0; i < redo_descriptions_.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << redo_descriptions_[i] << "\n";
            }
        }
        std::cout << "========================\n\n";
    }

    // ============================================================================
    // State Snapshot System (Memento Pattern)
    // ============================================================================

    template <typename T> inline std::unordered_map<std::string, Any> captureState(T *object) {
        const auto                          &meta = Registry::instance().get<T>();
        std::unordered_map<std::string, Any> state;

        for (const auto &field_name : meta.fields()) {
            state[field_name] = meta.get_field(*object, field_name);
        }

        return state;
    }

    template <typename T>
    inline void restoreState(T *object, const std::unordered_map<std::string, Any> &state) {
        const auto &meta = Registry::instance().get<T>();

        for (const auto &[field_name, value] : state) {
            try {
                meta.set_field(*object, field_name, value);
            } catch (const std::exception &e) {
                std::cerr << "Failed to restore field '" << field_name << "': " << e.what() << "\n";
            }
        }
    }

    class ObjectMemento {
    public:
        template <typename T>
        static ObjectMemento create(T *object, const std::string &label = "") {
            ObjectMemento memento;
            memento.state_     = captureState(object);
            memento.type_info_ = &typeid(T);
            memento.label_     = label;
            return memento;
        }

        template <typename T> void restore(T *object) const {
            if (type_info_ != &typeid(T)) {
                throw std::runtime_error("Type mismatch in memento restore");
            }
            restoreState(object, state_);
        }

        const std::string &label() const { return label_; }

    private:
        std::unordered_map<std::string, Any> state_;
        const std::type_info                *type_info_;
        std::string                          label_;
    };

    template <typename T>
    inline void HistoryManager::saveState(T *object, const std::string &label) {
        // Remove any states after current position
        history_.erase(history_.begin() + current_index_ + 1, history_.end());

        // Save new state
        history_.push_back(ObjectMemento::create(object, label));
        current_index_ = history_.size() - 1;

        if (!label.empty()) {
            std::cout << "[SNAPSHOT] " << label << "\n";
        }
    }

    template <typename T> inline void HistoryManager::undo(T *object) {
        if (current_index_ <= 0) {
            std::cout << "Cannot undo - at initial state\n";
            return;
        }

        current_index_--;
        history_[current_index_].restore(object);

        std::cout << "[UNDO SNAPSHOT]";
        if (!history_[current_index_].label().empty()) {
            std::cout << " Back to: " << history_[current_index_].label();
        }
        std::cout << "\n";
    }

    template <typename T> inline void HistoryManager::redo(T *object) {
        if (current_index_ >= static_cast<int>(history_.size()) - 1) {
            std::cout << "Cannot redo - at latest state\n";
            return;
        }

        current_index_++;
        history_[current_index_].restore(object);

        std::cout << "[REDO SNAPSHOT]";
        if (!history_[current_index_].label().empty()) {
            std::cout << " Forward to: " << history_[current_index_].label();
        }
        std::cout << "\n";
    }

    inline bool HistoryManager::canUndo() const {
        return current_index_ > 0;
    }
    inline bool HistoryManager::canRedo() const {
        return current_index_ < static_cast<int>(history_.size()) - 1;
    }

    inline void HistoryManager::printHistory() const {
        std::cout << "\n=== Snapshot History ===\n";
        std::cout << "Total snapshots: " << history_.size() << "\n";
        std::cout << "Current index: " << current_index_ << "\n";

        for (size_t i = 0; i < history_.size(); ++i) {
            std::cout << (i == static_cast<size_t>(current_index_) ? " > " : "   ");
            std::cout << i << ". " << history_[i].label() << "\n";
        }
        std::cout << "=======================\n\n";
    }

} // namespace rosetta::core
