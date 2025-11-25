// ============================================================================
// Qt6 Property Editor - Advanced Widget Customization
// ============================================================================
#pragma once

#include "qt_property_editor.h"
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QSlider>

namespace rosetta::qt {

    // ============================================================================
    // Custom Widget Creators for common types
    // ============================================================================

    /**
     * @brief Register a slider widget for bounded integer values
     * @param min Minimum value
     * @param max Maximum value
     */
    void register_int_slider(int min = 0, int max = 100);

    /**
     * @brief Register a combo box widget for enum-like integer values
     * @param options List of display names for each value (index = value)
     */
    void register_int_combo(const std::vector<std::string> &options);

    /**
     * @brief Register a file path selector widget
     * @param filter File filter (e.g., "Images (*.png *.jpg)")
     * @param directory If true, select directory instead of file
     */
    void register_path_selector(const std::string &filter = "All Files (*.*)", bool directory = false);

    /**
     * @brief Register a multiline text editor
     */
    inline void register_text_area();

    // ============================================================================
    // Type-specific Widget Registration Helpers
    // ============================================================================

    /**
     * @brief Register a custom widget for a specific field type
     *
     * Usage:
     *   register_widget_for_type<MyColor>([](auto getter, auto setter, auto on_change) {
     *       // Create and return custom widget
     *   });
     */
    template <typename T>
    void register_widget_for_type(
        std::function<QWidget *(const std::string &, std::function<core::Any()>, std::function<void(const core::Any &)>, PropertyChangedCallback)> creator);

    // ============================================================================
    // Property Editor Builder - Fluent API for configuration
    // ============================================================================

    template <typename T> class PropertyEditorBuilder {
    public:
        explicit PropertyEditorBuilder(T *object);

        /**
         * @brief Set a field to read-only
         */
        PropertyEditorBuilder &readOnly(const std::string &field_name);

        /**
         * @brief Hide a field
         */
        PropertyEditorBuilder &hide(const std::string &field_name);

        /**
         * @brief Group fields together
         */
        PropertyEditorBuilder &group(const std::string &group_name, const std::vector<std::string> &fields);

        /**
         * @brief Set display label for a field
         */
        PropertyEditorBuilder &label(const std::string &field_name, const std::string &display_label);

        /**
         * @brief Set tooltip for a field
         */
        PropertyEditorBuilder &tooltip(const std::string &field_name, const std::string &tooltip_text);

        /**
         * @brief Set numeric range constraint
         */
        template <typename ValueType>
        PropertyEditorBuilder &range(const std::string &field_name, ValueType min_val, ValueType max_val);

        /**
         * @brief Set change callback
         */
        PropertyEditorBuilder &onChange(PropertyChangedCallback callback);

        /**
         * @brief Build and return the editor
         */
        PropertyEditor<T> *build();

    private:
        T                                           *object_;
        PropertyEditor<T>                           *editor_;
        std::vector<std::string>                     hidden_fields_;
        std::unordered_map<std::string, std::string> labels_;
        std::unordered_map<std::string, std::string> tooltips_;
    };

    /**
     * @brief Convenience function to create a property editor builder
     */
    template <typename T> PropertyEditorBuilder<T> makePropertyEditor(T *object);
    template <typename T> PropertyEditorBuilder<T> makePropertyEditor(T &object);

    // ============================================================================
    // Property Binding - Two-way data binding between objects
    // ============================================================================

    /**
     * @brief Bind a property between two objects
     *
     * When source.field changes, target.field is updated automatically
     */
    template <typename SourceType, typename TargetType> class PropertyBinding {
    public:
        PropertyBinding(SourceType *source, const std::string &source_field, TargetType *target, const std::string &target_field, bool bidirectional = false);

        /**
         * @brief Sync target from source
         */
        void syncToTarget();

        /**
         * @brief Sync source from target (if bidirectional)
         */
        void syncToSource();

    private:
        SourceType                            *source_;
        TargetType                            *target_;
        std::string                            source_field_;
        std::string                            target_field_;
        bool                                   bidirectional_;
        const core::ClassMetadata<SourceType> *source_meta_;
        const core::ClassMetadata<TargetType> *target_meta_;
    };

} // namespace rosetta::qt

#include "inline/qt_property_editor_advanced.hxx"