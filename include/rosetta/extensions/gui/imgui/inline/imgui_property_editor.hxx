// ============================================================================
// ImGui Automatic Property Editor - Implementation
// ============================================================================
#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

namespace rosetta::imgui {

    // ============================================================================
    // WidgetDrawer Implementation
    // ============================================================================

    inline WidgetDrawer &WidgetDrawer::instance() {
        static WidgetDrawer drawer;
        return drawer;
    }

    inline WidgetDrawer::WidgetDrawer() {
        register_default_drawers();
    }

    inline void WidgetDrawer::register_drawer(std::type_index type, DrawerFunc drawer) {
        drawers_[type] = std::move(drawer);
    }

    inline bool WidgetDrawer::has_drawer(std::type_index type) const {
        return drawers_.find(type) != drawers_.end();
    }

    inline bool WidgetDrawer::draw_widget(
        std::type_index type,
        const std::string &label,
        std::function<core::Any()> getter,
        std::function<void(const core::Any &)> setter,
        PropertyChangedCallback on_change) {
        
        auto it = drawers_.find(type);
        if (it != drawers_.end()) {
            return it->second(label, getter, setter, on_change);
        }

        // Fallback: show unsupported type label
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
                          "%s: <unsupported type>", label.c_str());
        return false;
    }

    inline void WidgetDrawer::register_default_drawers() {
        // ----------------------------------------------------------------
        // Boolean
        // ----------------------------------------------------------------
        drawers_[std::type_index(typeid(bool))] = 
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            bool value = getter().as<bool>();
            if (ImGui::Checkbox(label.c_str(), &value)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        // ----------------------------------------------------------------
        // Integer types
        // ----------------------------------------------------------------
        drawers_[std::type_index(typeid(int))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = getter().as<int>();
            if (ImGui::DragInt(label.c_str(), &value, 1.0f)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(unsigned int))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<unsigned int>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, 0, INT_MAX)) {
                setter(core::Any(static_cast<unsigned int>(std::max(0, value))));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(long))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<long>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f)) {
                setter(core::Any(static_cast<long>(value)));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(long long))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            // ImGui doesn't have DragInt64, so we use a workaround with InputScalar
            long long value = getter().as<long long>();
            if (ImGui::InputScalar(label.c_str(), ImGuiDataType_S64, &value)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(unsigned long))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<unsigned long>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, 0, INT_MAX)) {
                setter(core::Any(static_cast<unsigned long>(std::max(0, value))));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(unsigned long long))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            unsigned long long value = getter().as<unsigned long long>();
            if (ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &value)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(size_t))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<size_t>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, 0, INT_MAX)) {
                setter(core::Any(static_cast<size_t>(std::max(0, value))));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(short))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<short>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, SHRT_MIN, SHRT_MAX)) {
                setter(core::Any(static_cast<short>(value)));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(unsigned short))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<unsigned short>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, 0, USHRT_MAX)) {
                setter(core::Any(static_cast<unsigned short>(std::max(0, value))));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(char))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<char>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, CHAR_MIN, CHAR_MAX)) {
                setter(core::Any(static_cast<char>(value)));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(unsigned char))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            int value = static_cast<int>(getter().as<unsigned char>());
            if (ImGui::DragInt(label.c_str(), &value, 1.0f, 0, UCHAR_MAX)) {
                setter(core::Any(static_cast<unsigned char>(std::max(0, value))));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        // ----------------------------------------------------------------
        // Floating point types
        // ----------------------------------------------------------------
        drawers_[std::type_index(typeid(float))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            float value = getter().as<float>();
            if (ImGui::DragFloat(label.c_str(), &value, 0.1f, 0.0f, 0.0f, "%.3f")) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(double))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            double value = getter().as<double>();
            float float_value = static_cast<float>(value);
            if (ImGui::DragFloat(label.c_str(), &float_value, 0.1f, 0.0f, 0.0f, "%.6f")) {
                setter(core::Any(static_cast<double>(float_value)));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        // ----------------------------------------------------------------
        // String types
        // ----------------------------------------------------------------
        drawers_[std::type_index(typeid(std::string))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            std::string value = getter().as<std::string>();
            char buffer[1024];
            std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer))) {
                setter(core::Any(std::string(buffer)));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        // Multiline string (for longer text)
        // Users can register this separately if needed with a different type wrapper

        // ----------------------------------------------------------------
        // Common math types - glm style vectors (3 floats)
        // ----------------------------------------------------------------
        // Note: Users should register their own vec types. Example provided here
        // for std::array<float, 3> which is commonly used

        drawers_[std::type_index(typeid(std::array<float, 2>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<float, 2>>();
            if (ImGui::DragFloat2(label.c_str(), value.data(), 0.1f)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(std::array<float, 3>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<float, 3>>();
            if (ImGui::DragFloat3(label.c_str(), value.data(), 0.1f)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(std::array<float, 4>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<float, 4>>();
            if (ImGui::DragFloat4(label.c_str(), value.data(), 0.1f)) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(std::array<int, 2>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<int, 2>>();
            if (ImGui::DragInt2(label.c_str(), value.data())) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(std::array<int, 3>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<int, 3>>();
            if (ImGui::DragInt3(label.c_str(), value.data())) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };

        drawers_[std::type_index(typeid(std::array<int, 4>))] =
            [](const std::string &label, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback on_change) -> bool {
            auto value = getter().as<std::array<int, 4>>();
            if (ImGui::DragInt4(label.c_str(), value.data())) {
                setter(core::Any(value));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        };
    }

    // ============================================================================
    // PropertyEditor Implementation
    // ============================================================================

    template <typename T>
    PropertyEditor<T>::PropertyEditor(T *object) : object_(object) {
        if (object_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
    }

    template <typename T>
    PropertyEditor<T>::PropertyEditor(T &object) : PropertyEditor(&object) {}

    template <typename T>
    void PropertyEditor<T>::set_object(T *object) {
        object_ = object;
        if (object_ && !metadata_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
        fields_organized_ = false;
    }

    template <typename T>
    void PropertyEditor<T>::set_object(T &object) {
        set_object(&object);
    }

    template <typename T>
    void PropertyEditor<T>::set_on_change(PropertyChangedCallback callback) {
        on_change_callback_ = std::move(callback);
    }

    template <typename T>
    void PropertyEditor<T>::set_read_only(bool readonly) {
        read_only_ = readonly;
    }

    template <typename T>
    void PropertyEditor<T>::set_field_visible(const std::string &field_name, bool visible) {
        field_visibility_[field_name] = visible;
    }

    template <typename T>
    bool PropertyEditor<T>::is_field_visible(const std::string &field_name) const {
        auto it = field_visibility_.find(field_name);
        return it == field_visibility_.end() || it->second;
    }

    template <typename T>
    void PropertyEditor<T>::set_field_group(const std::string &field_name, 
                                            const std::string &group_name) {
        field_groups_[field_name] = group_name;
        fields_organized_ = false;
    }

    template <typename T>
    void PropertyEditor<T>::set_field_display_name(const std::string &field_name,
                                                    const std::string &display_name) {
        field_display_names_[field_name] = display_name;
    }

    template <typename T>
    std::string PropertyEditor<T>::get_display_name(const std::string &field_name) const {
        auto it = field_display_names_.find(field_name);
        if (it != field_display_names_.end()) {
            return it->second;
        }
        // Convert snake_case or camelCase to Title Case
        std::string result;
        bool capitalize_next = true;
        for (char c : field_name) {
            if (c == '_') {
                result += ' ';
                capitalize_next = true;
            } else if (std::isupper(c) && !result.empty() && result.back() != ' ') {
                result += ' ';
                result += c;
                capitalize_next = false;
            } else {
                result += capitalize_next ? static_cast<char>(std::toupper(c)) : c;
                capitalize_next = false;
            }
        }
        return result;
    }

    template <typename T>
    void PropertyEditor<T>::organize_fields_into_groups() {
        if (!metadata_) return;

        ungrouped_fields_.clear();
        grouped_fields_.clear();

        for (const auto &field_name : metadata_->fields()) {
            auto it = field_groups_.find(field_name);
            if (it != field_groups_.end()) {
                grouped_fields_[it->second].push_back(field_name);
            } else {
                ungrouped_fields_.push_back(field_name);
            }
        }

        fields_organized_ = true;
    }

    template <typename T>
    void PropertyEditor<T>::draw(const char *title) {
        if (!object_ || !metadata_) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No object to edit");
            return;
        }

        if (title && title[0] != '\0') {
            if (ImGui::Begin(title)) {
                draw_fields();
            }
            ImGui::End();
        } else {
            draw_fields();
        }
    }

    template <typename T>
    void PropertyEditor<T>::draw_fields() {
        if (!object_ || !metadata_) return;

        if (!fields_organized_) {
            organize_fields_into_groups();
        }

        // Header with class name
        ImGui::TextColored(config_.group_header_color, "Class: %s", metadata_->name().c_str());
        ImGui::Separator();

        if (read_only_) {
            ImGui::BeginDisabled();
        }

        // Draw ungrouped fields first
        for (const auto &field_name : ungrouped_fields_) {
            if (is_field_visible(field_name)) {
                draw_field(field_name);
            }
        }

        // Draw grouped fields
        for (const auto &[group_name, fields] : grouped_fields_) {
            draw_group(group_name, fields);
        }

        if (read_only_) {
            ImGui::EndDisabled();
        }
    }

    template <typename T>
    void PropertyEditor<T>::draw_field(const std::string &field_name) {
        std::type_index type = metadata_->get_field_type(field_name);
        std::string display_name = get_display_name(field_name);

        // Create getter/setter lambdas
        auto getter = [this, &field_name]() -> core::Any {
            return metadata_->get_field(*object_, field_name);
        };

        auto setter = [this, &field_name](const core::Any &value) {
            metadata_->set_field(*object_, field_name, value);
        };

        // Push unique ID to avoid conflicts
        ImGui::PushID(field_name.c_str());

        // Draw the widget
        WidgetDrawer::instance().draw_widget(type, display_name, getter, setter, 
                                              on_change_callback_);

        // Show type hint tooltip if enabled
        if (config_.show_type_hints && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Type: %s\nField: %s", 
                             core::get_readable_type_name(type).c_str(),
                             field_name.c_str());
        }

        ImGui::PopID();
    }

    template <typename T>
    void PropertyEditor<T>::draw_group(const std::string &group_name,
                                       const std::vector<std::string> &fields) {
        // Initialize group open state
        if (group_open_state_.find(group_name) == group_open_state_.end()) {
            group_open_state_[group_name] = config_.default_groups_open;
        }

        bool &is_open = group_open_state_[group_name];

        if (config_.collapsible_groups) {
            ImGui::PushStyleColor(ImGuiCol_Header, config_.group_header_color);
            is_open = ImGui::CollapsingHeader(group_name.c_str(), 
                                              config_.default_groups_open 
                                              ? ImGuiTreeNodeFlags_DefaultOpen 
                                              : 0);
            ImGui::PopStyleColor();

            if (is_open) {
                ImGui::Indent(config_.indent_width);
                for (const auto &field_name : fields) {
                    if (is_field_visible(field_name)) {
                        draw_field(field_name);
                    }
                }
                ImGui::Unindent(config_.indent_width);
            }
        } else {
            // Non-collapsible group header
            ImGui::TextColored(config_.group_header_color, "%s", group_name.c_str());
            ImGui::Separator();
            ImGui::Indent(config_.indent_width);
            for (const auto &field_name : fields) {
                if (is_field_visible(field_name)) {
                    draw_field(field_name);
                }
            }
            ImGui::Unindent(config_.indent_width);
        }
    }

    // ============================================================================
    // MethodInvoker Implementation
    // ============================================================================

    template <typename T>
    MethodInvoker<T>::MethodInvoker(T *object) : object_(object) {
        if (object_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
    }

    template <typename T>
    void MethodInvoker<T>::set_object(T *object) {
        object_ = object;
        if (object_ && !metadata_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
    }

    template <typename T>
    void MethodInvoker<T>::set_on_invoke(std::function<void(const std::string &)> callback) {
        on_invoke_callback_ = std::move(callback);
    }

    template <typename T>
    void MethodInvoker<T>::set_method_filter(std::function<bool(const std::string &)> filter) {
        method_filter_ = std::move(filter);
    }

    template <typename T>
    void MethodInvoker<T>::draw(const char *title) {
        if (!object_ || !metadata_) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No object");
            return;
        }

        if (title && title[0] != '\0') {
            if (ImGui::Begin(title)) {
                draw_methods();
            }
            ImGui::End();
        } else {
            draw_methods();
        }
    }

    template <typename T>
    void MethodInvoker<T>::draw_methods() {
        if (!object_ || !metadata_) return;

        ImGui::Text("Methods:");
        ImGui::Separator();

        for (const auto &method_name : metadata_->methods()) {
            // Apply filter if set
            if (method_filter_ && !method_filter_(method_name)) {
                continue;
            }

            draw_method_button(method_name);
        }

        // Handle argument input popup
        if (show_args_popup_) {
            show_argument_popup(current_method_);
        }
    }

    template <typename T>
    bool MethodInvoker<T>::draw_method_button(const std::string &method_name) {
        const auto &info_list = metadata_->method_info(method_name);
        if (info_list.empty()) return false;

        // Use the first overload for now
        const auto &info = info_list[0];

        ImGui::PushID(method_name.c_str());

        // Build signature string
        std::string signature;
        if (config_.show_method_signatures) {
            signature = "(";
            for (size_t i = 0; i < info.arg_types.size(); ++i) {
                if (i > 0) signature += ", ";
                signature += core::get_readable_type_name(info.arg_types[i]);
            }
            signature += ")";
        }

        bool invoked = false;

        if (info.arity == 0) {
            // No arguments - simple button
            if (ImGui::Button(method_name.c_str())) {
                try {
                    metadata_->invoke_method(*object_, method_name, {});
                    if (on_invoke_callback_) {
                        on_invoke_callback_(method_name);
                    }
                    invoked = true;
                } catch (const std::exception &e) {
                    ImGui::OpenPopup("Method Error");
                }
            }
        } else {
            // Has arguments - show button that opens popup
            if (ImGui::Button(method_name.c_str())) {
                current_method_ = method_name;
                pending_args_.clear();
                pending_args_.resize(info.arity);
                show_args_popup_ = true;
                ImGui::OpenPopup("Method Arguments");
            }
        }

        // Show signature hint
        if (config_.show_method_signatures && !signature.empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", signature.c_str());
        }

        // Error popup
        if (ImGui::BeginPopupModal("Method Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Method invocation failed!");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();

        return invoked;
    }

    template <typename T>
    bool MethodInvoker<T>::show_argument_popup(const std::string &method_name) {
        bool invoked = false;

        if (ImGui::BeginPopupModal("Method Arguments", &show_args_popup_, 
                                    ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Arguments for %s:", method_name.c_str());
            ImGui::Separator();

            const auto &info_list = metadata_->method_info(method_name);
            if (!info_list.empty()) {
                const auto &info = info_list[0];

                // Create input widgets for each argument
                for (size_t i = 0; i < info.arg_types.size(); ++i) {
                    std::type_index type = info.arg_types[i];
                    std::string label = "Arg " + std::to_string(i) + " (" + 
                                       core::get_readable_type_name(type) + ")";
                    ImGui::PushID(static_cast<int>(i));

                    // Initialize default values if needed
                    if (!pending_args_[i].has_value()) {
                        if (type == std::type_index(typeid(int))) {
                            pending_args_[i] = core::Any(0);
                        } else if (type == std::type_index(typeid(float))) {
                            pending_args_[i] = core::Any(0.0f);
                        } else if (type == std::type_index(typeid(double))) {
                            pending_args_[i] = core::Any(0.0);
                        } else if (type == std::type_index(typeid(bool))) {
                            pending_args_[i] = core::Any(false);
                        } else if (type == std::type_index(typeid(std::string))) {
                            pending_args_[i] = core::Any(std::string(""));
                        }
                    }

                    // Draw appropriate input
                    if (type == std::type_index(typeid(int))) {
                        int val = pending_args_[i].template as<int>();
                        if (ImGui::DragInt(label.c_str(), &val)) {
                            pending_args_[i] = core::Any(val);
                        }
                    } else if (type == std::type_index(typeid(float))) {
                        float val = pending_args_[i].template as<float>();
                        if (ImGui::DragFloat(label.c_str(), &val, 0.1f)) {
                            pending_args_[i] = core::Any(val);
                        }
                    } else if (type == std::type_index(typeid(double))) {
                        float val = static_cast<float>(pending_args_[i].template as<double>());
                        if (ImGui::DragFloat(label.c_str(), &val, 0.1f)) {
                            pending_args_[i] = core::Any(static_cast<double>(val));
                        }
                    } else if (type == std::type_index(typeid(bool))) {
                        bool val = pending_args_[i].template as<bool>();
                        if (ImGui::Checkbox(label.c_str(), &val)) {
                            pending_args_[i] = core::Any(val);
                        }
                    } else if (type == std::type_index(typeid(std::string))) {
                        std::string str = pending_args_[i].template as<std::string>();
                        char buffer[256];
                        std::strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
                        buffer[sizeof(buffer) - 1] = '\0';
                        if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer))) {
                            pending_args_[i] = core::Any(std::string(buffer));
                        }
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), 
                                          "Unsupported argument type");
                    }

                    ImGui::PopID();
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Invoke", ImVec2(120, 0))) {
                try {
                    metadata_->invoke_method(*object_, method_name, pending_args_);
                    if (on_invoke_callback_) {
                        on_invoke_callback_(method_name);
                    }
                    invoked = true;
                    show_args_popup_ = false;
                    ImGui::CloseCurrentPopup();
                } catch (const std::exception &e) {
                    // Keep popup open, show error
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                show_args_popup_ = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return invoked;
    }

    // ============================================================================
    // ObjectInspector Implementation
    // ============================================================================

    template <typename T>
    ObjectInspector<T>::ObjectInspector(T *object)
        : object_(object), property_editor_(object), method_invoker_(object) {
        // Connect method invocation to property refresh
        method_invoker_.set_on_invoke([this](const std::string &) {
            // Properties may have changed after method call
            // ImGui doesn't need explicit refresh - next frame will redraw
        });
    }

    template <typename T>
    void ObjectInspector<T>::set_object(T *object) {
        object_ = object;
        property_editor_.set_object(object);
        method_invoker_.set_object(object);
    }

    template <typename T>
    void ObjectInspector<T>::draw(const char *title) {
        if (ImGui::Begin(title)) {
            if (!object_) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No object selected");
            } else {
                if (ImGui::BeginTabBar("InspectorTabs")) {
                    if (ImGui::BeginTabItem("Properties")) {
                        property_editor_.draw_fields();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Methods")) {
                        method_invoker_.draw_methods();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
        }
        ImGui::End();
    }

    // ============================================================================
    // MultiObjectPropertyEditor Implementation
    // ============================================================================

    template <typename T>
    void MultiObjectPropertyEditor<T>::set_objects(const std::vector<T *> &objects) {
        objects_ = objects;
        if (!objects_.empty() && !metadata_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::add_object(T *object) {
        objects_.push_back(object);
        if (!metadata_) {
            metadata_ = &core::Registry::instance().get<T>();
        }
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::remove_object(T *object) {
        objects_.erase(std::remove(objects_.begin(), objects_.end(), object), objects_.end());
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::clear() {
        objects_.clear();
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::set_on_change(PropertyChangedCallback callback) {
        on_change_callback_ = std::move(callback);
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::draw(const char *title) {
        if (objects_.empty() || !metadata_) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No objects to edit");
            return;
        }

        if (title && title[0] != '\0') {
            if (ImGui::Begin(title)) {
                draw_fields();
            }
            ImGui::End();
        } else {
            draw_fields();
        }
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::draw_fields() {
        if (objects_.empty() || !metadata_) return;

        ImGui::Text("Editing %zu objects", objects_.size());
        ImGui::Separator();

        for (const auto &field_name : metadata_->fields()) {
            draw_field(field_name);
        }
    }

    template <typename T>
    bool MultiObjectPropertyEditor<T>::all_objects_have_same_value(
        const std::string &field_name) const {
        if (objects_.size() <= 1) return true;

        core::Any first_value = metadata_->get_field(*objects_[0], field_name);
        for (size_t i = 1; i < objects_.size(); ++i) {
            core::Any value = metadata_->get_field(*objects_[i], field_name);
            // Simple string comparison - works for most types
            if (first_value.toString() != value.toString()) {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::draw_field(const std::string &field_name) {
        std::type_index type = metadata_->get_field_type(field_name);

        ImGui::PushID(field_name.c_str());

        if (all_objects_have_same_value(field_name)) {
            // All objects have same value - show normal editor
            auto getter = [this, &field_name]() -> core::Any {
                return metadata_->get_field(*objects_[0], field_name);
            };

            auto setter = [this, &field_name](const core::Any &value) {
                for (T *obj : objects_) {
                    metadata_->set_field(*obj, field_name, value);
                }
            };

            WidgetDrawer::instance().draw_widget(type, field_name, getter, setter,
                                                  on_change_callback_);
        } else {
            // Mixed values - show special indicator
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), 
                              "%s: <mixed values>", field_name.c_str());
            
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Objects have different values for this field");
            }
        }

        ImGui::PopID();
    }

    // ============================================================================
    // Utility Functions Implementation
    // ============================================================================

    template <typename T>
    bool quick_edit(const char *label, T &object) {
        static std::unordered_map<const void *, PropertyEditor<T>> editors;
        
        auto &editor = editors[&object];
        editor.set_object(&object);
        
        bool changed = false;
        if (ImGui::TreeNode(label)) {
            // Save the old on_change to detect modifications
            bool modified = false;
            editor.set_on_change([&modified](const std::string &) {
                modified = true;
            });
            
            editor.draw_fields();
            changed = modified;
            
            ImGui::TreePop();
        }
        return changed;
    }

    // ============================================================================
    // Widget Helper Functions
    // ============================================================================

    namespace widgets {

        inline bool draw_bool(const char *label, bool *value) {
            return ImGui::Checkbox(label, value);
        }

        inline bool draw_int(const char *label, int *value, int min, int max, float speed) {
            if (min == 0 && max == 0) {
                return ImGui::DragInt(label, value, speed);
            }
            return ImGui::SliderInt(label, value, min, max);
        }

        inline bool draw_float(const char *label, float *value, float min, float max, float speed) {
            if (min == 0.0f && max == 0.0f) {
                return ImGui::DragFloat(label, value, speed, 0.0f, 0.0f, "%.3f");
            }
            return ImGui::SliderFloat(label, value, min, max, "%.3f");
        }

        inline bool draw_double(const char *label, double *value, double min, double max, float speed) {
            float f = static_cast<float>(*value);
            bool changed = false;
            if (min == 0.0 && max == 0.0) {
                changed = ImGui::DragFloat(label, &f, speed, 0.0f, 0.0f, "%.6f");
            } else {
                changed = ImGui::SliderFloat(label, &f, static_cast<float>(min), 
                                             static_cast<float>(max), "%.6f");
            }
            if (changed) {
                *value = static_cast<double>(f);
            }
            return changed;
        }

        inline bool draw_string(const char *label, std::string *value, size_t max_length) {
            std::vector<char> buffer(max_length);
            std::strncpy(buffer.data(), value->c_str(), max_length - 1);
            buffer[max_length - 1] = '\0';
            
            if (ImGui::InputText(label, buffer.data(), max_length)) {
                *value = buffer.data();
                return true;
            }
            return false;
        }

        inline bool draw_vec2(const char *label, float *v, float speed) {
            return ImGui::DragFloat2(label, v, speed);
        }

        inline bool draw_vec3(const char *label, float *v, float speed) {
            return ImGui::DragFloat3(label, v, speed);
        }

        inline bool draw_vec4(const char *label, float *v, float speed) {
            return ImGui::DragFloat4(label, v, speed);
        }

        inline bool draw_color3(const char *label, float *col) {
            return ImGui::ColorEdit3(label, col);
        }

        inline bool draw_color4(const char *label, float *col) {
            return ImGui::ColorEdit4(label, col);
        }

        inline bool draw_angle(const char *label, float *rad) {
            float deg = *rad * (180.0f / 3.14159265359f);
            if (ImGui::DragFloat(label, &deg, 1.0f, -360.0f, 360.0f, "%.1f°")) {
                *rad = deg * (3.14159265359f / 180.0f);
                return true;
            }
            return false;
        }

        inline bool draw_enum_combo(const char *label, int *value, 
                                    const char *const *items, int item_count) {
            return ImGui::Combo(label, value, items, item_count);
        }

    } // namespace widgets

} // namespace rosetta::imgui
