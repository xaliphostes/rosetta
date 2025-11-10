namespace rosetta::core {

    inline Registry &Registry::instance() {
        static Registry reg;
        return reg;
    }

    template <typename Class>
    inline ClassMetadata<Class> &Registry::register_class(const std::string &name) {
        auto  holder                  = std::make_unique<MetadataHolderImpl<Class>>(name);
        auto *ptr                     = &holder->metadata;
        classes_[name]                = std::move(holder);
        type_to_name_[&typeid(Class)] = name;
        return *ptr;
    }

    template <typename Class> inline ClassMetadata<Class> &Registry::get() {
        for (auto &[name, holder] : classes_) {
            if (auto *typed = dynamic_cast<MetadataHolderImpl<Class> *>(holder.get())) {
                return typed->metadata;
            }
        }
        throw std::runtime_error("Class not registered: " + std::string(typeid(Class).name()));
    }

    template <typename Class> inline const ClassMetadata<Class> &Registry::get() const {
        for (const auto &[name, holder] : classes_) {
            if (auto *typed = dynamic_cast<const MetadataHolderImpl<Class> *>(holder.get())) {
                return typed->metadata;
            }
        }
        throw std::runtime_error("Class not registered: " + std::string(typeid(Class).name()));
    }

    inline const Registry::MetadataHolder *Registry::get_by_name(const std::string &name) const {
        auto it = classes_.find(name);
        return it != classes_.end() ? it->second.get() : nullptr;
    }

    inline std::string Registry::get_class_name(const std::type_info &type) const {
        auto it = type_to_name_.find(&type);
        return it != type_to_name_.end() ? it->second : "";
    }

    inline std::vector<std::string> Registry::list_classes() const {
        std::vector<std::string> names;
        names.reserve(classes_.size());
        for (const auto &[name, _] : classes_) {
            names.push_back(name);
        }
        return names;
    }

    inline bool Registry::has_class(const std::string &name) const {
        return classes_.find(name) != classes_.end();
    }

    template <typename Class> inline bool Registry::has_class() const {
        return type_to_name_.find(&typeid(Class)) != type_to_name_.end();
    }

    inline size_t Registry::size() const {
        return classes_.size();
    }

    inline void Registry::clear() {
        classes_.clear();
        type_to_name_.clear();
    }

} // namespace rosetta::core