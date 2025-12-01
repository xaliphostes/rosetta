// ============================================================================
// ImGui Automatic Property Editor using Rosetta introspection
// ============================================================================
#pragma once

#include <imgui.h>

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

// Forward declarations - in real usage, include your Rosetta headers
#include <rosetta/rosetta.h>

namespace rosetta::imgui {

    // ============================================================================
    // Property Change Callback
    // ============================================================================

    using PropertyChangedCallback = std::function<void(const std::string &field_name)>;

    // ============================================================================
    // Widget Drawer - Draws appropriate widgets for different types
    // ============================================================================

    /**
     * @brief Factory class for creating ImGui widgets based on type
     *
     * Provides automatic widget selection for common types and allows
     * registration of custom widget drawers for user-defined types.
     */
    class WidgetDrawer {
    public:
        /**
         * @brief Widget drawer function signature
         * @param label Field display name
         * @param getter Function to get the current value
         * @param setter Function to set a new value
         * @param on_change Optional callback when value changes
         * @return true if value was modified
         */
        using DrawerFunc = std::function<bool(
            const std::string &label, std::function<core::Any()> getter,
            std::function<void(const core::Any &)> setter, PropertyChangedCallback on_change)>;

        static WidgetDrawer &instance();

        /**
         * @brief Register a custom widget drawer for a specific type
         */
        void register_drawer(std::type_index type, DrawerFunc drawer);

        /**
         * @brief Draw a widget for a given type
         * @return true if value was modified
         */
        bool draw_widget(std::type_index type, const std::string &label,
                         std::function<core::Any()>             getter,
                         std::function<void(const core::Any &)> setter,
                         PropertyChangedCallback                on_change);

        /**
         * @brief Check if a drawer exists for a type
         */
        bool has_drawer(std::type_index type) const;

    private:
        WidgetDrawer();
        void register_default_drawers();

        std::unordered_map<std::type_index, DrawerFunc> drawers_;
    };

    // ============================================================================
    // Widget Configuration - Controls widget appearance and behavior
    // ============================================================================

    struct PropertyEditorConfig {
        // Layout settings
        float label_width         = 120.0f; // Width for field labels
        float indent_width        = 15.0f;  // Indentation for nested objects
        bool  show_type_hints     = false;  // Show type in tooltip
        bool  collapsible_groups  = true;   // Allow collapsing groups
        bool  default_groups_open = true;   // Groups open by default

        // Widget settings
        float drag_speed           = 0.1f;  // Speed for drag widgets
        int   decimal_precision    = 3;     // Decimal places for floats
        bool  use_sliders_for_ints = false; // Use sliders instead of drag
        int   slider_min           = 0;     // Default slider min
        int   slider_max           = 100;   // Default slider max

        // Color settings
        ImVec4 modified_color     = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Changed value
        ImVec4 readonly_color     = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Read-only field
        ImVec4 group_header_color = ImVec4(0.3f, 0.5f, 0.7f, 1.0f); // Group headers

        // Behavior
        bool confirm_method_calls   = false; // Confirm before calling methods
        bool show_method_signatures = true;  // Show method signatures
    };

    // ============================================================================
    // Property Editor - Main class for editing object properties
    // ============================================================================

    /**
     * @brief ImGui property editor that automatically generates UI from Rosetta metadata
     *
     * @example
     * ```cpp
     * // Register your class with Rosetta
     * Registry::instance()
     *     .register_class<Person>("Person")
     *     .field("name", &Person::name)
     *     .field("age", &Person::age)
     *     .field("height", &Person::height)
     *     .method("birthday", &Person::birthday);
     *
     * // Create and use the property editor
     * Person person{"Alice", 30, 1.65f};
     * rosetta::imgui::PropertyEditor<Person> editor(&person);
     * editor.set_on_change([](const std::string& field) {
     *     std::cout << "Changed: " << field << std::endl;
     * });
     *
     * // In your ImGui render loop:
     * editor.draw();
     * ```
     */
    template <typename T> class PropertyEditor {
    public:
        explicit PropertyEditor(T *object = nullptr);
        explicit PropertyEditor(T &object);

        /**
         * @brief Set the object to edit
         */
        void set_object(T *object);
        void set_object(T &object);

        /**
         * @brief Get the current object
         */
        T       *get_object() { return object_; }
        const T *get_object() const { return object_; }

        /**
         * @brief Draw the property editor UI
         * Call this in your ImGui render loop
         * @param title Optional window title (if empty, no window is created)
         */
        void draw(const char *title = nullptr);

        /**
         * @brief Draw only the field widgets (no window)
         */
        void draw_fields();

        /**
         * @brief Set callback for when any property changes
         */
        void set_on_change(PropertyChangedCallback callback);

        /**
         * @brief Enable/disable read-only mode
         */
        void set_read_only(bool readonly);
        bool is_read_only() const { return read_only_; }

        /**
         * @brief Show/hide specific fields
         */
        void set_field_visible(const std::string &field_name, bool visible);
        bool is_field_visible(const std::string &field_name) const;

        /**
         * @brief Group fields under a collapsible section
         */
        void set_field_group(const std::string &field_name, const std::string &group_name);

        /**
         * @brief Set custom display name for a field
         */
        void set_field_display_name(const std::string &field_name, const std::string &display_name);

        /**
         * @brief Access configuration
         */
        PropertyEditorConfig       &config() { return config_; }
        const PropertyEditorConfig &config() const { return config_; }

    private:
        void draw_field(const std::string &field_name);
        void draw_group(const std::string &group_name, const std::vector<std::string> &fields);
        std::string get_display_name(const std::string &field_name) const;
        void        organize_fields_into_groups();

        T                            *object_   = nullptr;
        const core::ClassMetadata<T> *metadata_ = nullptr;
        PropertyChangedCallback       on_change_callback_;
        PropertyEditorConfig          config_;
        bool                          read_only_ = false;

        std::unordered_map<std::string, bool>        field_visibility_;
        std::unordered_map<std::string, std::string> field_groups_;
        std::unordered_map<std::string, std::string> field_display_names_;
        std::unordered_map<std::string, bool>        group_open_state_;

        // Organized field lists
        std::vector<std::string>                                  ungrouped_fields_;
        std::unordered_map<std::string, std::vector<std::string>> grouped_fields_;
        bool                                                      fields_organized_ = false;
    };

    // ============================================================================
    // Method Invoker - Panel for invoking methods
    // ============================================================================

    /**
     * @brief ImGui panel for invoking methods on an object
     *
     * Automatically generates buttons for registered methods and handles
     * argument input dialogs for methods with parameters.
     */
    template <typename T> class MethodInvoker {
    public:
        explicit MethodInvoker(T *object = nullptr);

        void set_object(T *object);
        T   *get_object() { return object_; }

        /**
         * @brief Draw the method invoker UI
         * @param title Optional window title
         */
        void draw(const char *title = nullptr);

        /**
         * @brief Draw only the method buttons (no window)
         */
        void draw_methods();

        /**
         * @brief Set callback for after method is invoked
         */
        void set_on_invoke(std::function<void(const std::string &)> callback);

        /**
         * @brief Filter which methods to show
         */
        void set_method_filter(std::function<bool(const std::string &)> filter);

        /**
         * @brief Access configuration
         */
        PropertyEditorConfig &config() { return config_; }

    private:
        bool draw_method_button(const std::string &method_name);
        bool show_argument_popup(const std::string &method_name);

        T                                       *object_   = nullptr;
        const core::ClassMetadata<T>            *metadata_ = nullptr;
        std::function<void(const std::string &)> on_invoke_callback_;
        std::function<bool(const std::string &)> method_filter_;
        PropertyEditorConfig                     config_;

        // State for argument input popup
        std::string            current_method_;
        std::vector<core::Any> pending_args_;
        bool                   show_args_popup_ = false;
    };

    // ============================================================================
    // Object Inspector - Combined property editor + method invoker
    // ============================================================================

    /**
     * @brief Complete object inspector with tabbed property editor and method invoker
     */
    template <typename T> class ObjectInspector {
    public:
        explicit ObjectInspector(T *object = nullptr);

        void set_object(T *object);
        T   *get_object() { return object_; }

        /**
         * @brief Draw the inspector UI
         * @param title Window title
         */
        void draw(const char *title = "Object Inspector");

        /**
         * @brief Access sub-components
         */
        PropertyEditor<T> &property_editor() { return property_editor_; }
        MethodInvoker<T>  &method_invoker() { return method_invoker_; }

    private:
        T                *object_ = nullptr;
        PropertyEditor<T> property_editor_;
        MethodInvoker<T>  method_invoker_;
    };

    // ============================================================================
    // Multi-Object Property Editor - Edit multiple objects simultaneously
    // ============================================================================

    /**
     * @brief Property editor for editing multiple objects at once
     *
     * Shows fields that have the same value across all objects,
     * and indicates mixed values for fields that differ.
     */
    template <typename T> class MultiObjectPropertyEditor {
    public:
        MultiObjectPropertyEditor() = default;

        void set_objects(const std::vector<T *> &objects);
        void add_object(T *object);
        void remove_object(T *object);
        void clear();

        /**
         * @brief Draw the multi-object editor UI
         */
        void draw(const char *title = nullptr);

        /**
         * @brief Draw only the field widgets
         */
        void draw_fields();

        void set_on_change(PropertyChangedCallback callback);

    private:
        bool all_objects_have_same_value(const std::string &field_name) const;
        void draw_field(const std::string &field_name);

        std::vector<T *>              objects_;
        const core::ClassMetadata<T> *metadata_ = nullptr;
        PropertyChangedCallback       on_change_callback_;
        PropertyEditorConfig          config_;
    };

    // ============================================================================
    // Utility Functions
    // ============================================================================

    /**
     * @brief Quick function to draw property editor for any registered object
     */
    template <typename T> bool quick_edit(const char *label, T &object);

    /**
     * @brief Draw a single field widget (standalone)
     * @todo Ben TODO !
     */
    template <typename T> bool draw_field_widget(const char *label, T &value);

    // ============================================================================
    // Type-Specific Widget Helpers (for custom extensions)
    // ============================================================================

    namespace widgets {

        // Standard widgets
        bool draw_bool(const char *label, bool *value);
        bool draw_int(const char *label, int *value, int min = 0, int max = 0, float speed = 1.0f);
        bool draw_float(const char *label, float *value, float min = 0, float max = 0,
                        float speed = 0.1f);
        bool draw_double(const char *label, double *value, double min = 0, double max = 0,
                         float speed = 0.1f);
        bool draw_string(const char *label, std::string *value, size_t max_length = 256);

        // Vector widgets
        bool draw_vec2(const char *label, float *v, float speed = 0.1f);
        bool draw_vec3(const char *label, float *v, float speed = 0.1f);
        bool draw_vec4(const char *label, float *v, float speed = 0.1f);

        // Color widgets
        bool draw_color3(const char *label, float *col);
        bool draw_color4(const char *label, float *col);

        // Special widgets
        bool draw_angle(const char *label, float *rad); // Angle in radians, shown in degrees
        bool draw_enum_combo(const char *label, int *value, const char *const *items,
                             int item_count);

    } // namespace widgets

} // namespace rosetta::imgui

#include "inline/imgui_property_editor.hxx"
