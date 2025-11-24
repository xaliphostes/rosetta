// ============================================================================
// Qt6 Automatic Property Editor using Rosetta introspection
// ============================================================================
#pragma once

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

// Forward declarations - in real usage, include your Rosetta headers
#include <rosetta/rosetta.h>

namespace rosetta::qt {

    // ============================================================================
    // Property Change Callback
    // ============================================================================

    using PropertyChangedCallback = std::function<void(const std::string &field_name)>;

    // ============================================================================
    // Widget Factory - Creates appropriate widgets for different types
    // ============================================================================

    class WidgetFactory {
    public:
        using WidgetCreator = std::function<QWidget *(
            const std::string &field_name, std::function<core::Any()> getter,
            std::function<void(const core::Any &)> setter, PropertyChangedCallback on_change)>;

        static WidgetFactory &instance();

        /**
         * @brief Register a custom widget creator for a specific type
         */
        void register_widget(std::type_index type, WidgetCreator creator);

        /**
         * @brief Create a widget for a given type
         */
        QWidget *create_widget(std::type_index type, const std::string &field_name,
                               std::function<core::Any()>             getter,
                               std::function<void(const core::Any &)> setter,
                               PropertyChangedCallback                on_change);

        /**
         * @brief Check if a widget creator exists for a type
         */
        bool has_widget(std::type_index type) const;

    private:
        WidgetFactory();
        void register_default_widgets();

        std::unordered_map<std::type_index, WidgetCreator> creators_;
    };

    // ============================================================================
    // Property Editor Widget - Main widget for editing object properties
    // ============================================================================

    /**
     * @example
     * ```cpp
     * // Register your class with Rosetta
     * Registry::instance()
     *     .register_class<Person>("Person")
     *     .field("name", &Person::name)
     *     .field("age", &Person::age)
     *     .method("birthday", &Person::birthday);
     * 
     * // Create the property editor
     * Person person{"Alice", 30};
     * auto *editor = new rosetta::qt::PropertyEditor<Person>(&person);
     * editor->setPropertyChangedCallback([](const std::string& field) {
     *     qDebug() << "Changed:" << field.c_str();
     * });
     * ```
     */
    template <typename T> class PropertyEditor : public QWidget {
    public:
        explicit PropertyEditor(T *object, QWidget *parent = nullptr);
        explicit PropertyEditor(T &object, QWidget *parent = nullptr);

        /**
         * @brief Set the object to edit
         */
        void setObject(T *object);
        void setObject(T &object);

        /**
         * @brief Refresh all property widgets from the object
         */
        void refresh();

        /**
         * @brief Set callback for when any property changes
         */
        void setPropertyChangedCallback(PropertyChangedCallback callback);

        /**
         * @brief Enable/disable read-only mode
         */
        void setReadOnly(bool readonly);

        /**
         * @brief Show/hide specific fields
         */
        void setFieldVisible(const std::string &field_name, bool visible);

        /**
         * @brief Group fields under a collapsible section
         */
        void setFieldGroup(const std::string &field_name, const std::string &group_name);

    private:
        void setupUi();
        void createFieldWidgets();
        void createMethodButtons();

        T                                             *object_      = nullptr;
        const core::ClassMetadata<T>                  *metadata_    = nullptr;
        QVBoxLayout                                   *main_layout_ = nullptr;
        QFormLayout                                   *form_layout_ = nullptr;
        QScrollArea                                   *scroll_area_ = nullptr;
        PropertyChangedCallback                        on_change_callback_;
        bool                                           read_only_ = false;
        std::unordered_map<std::string, QWidget *>     field_widgets_;
        std::unordered_map<std::string, std::string>   field_groups_;
        std::unordered_map<std::string, QGroupBox *>   group_boxes_;
        std::unordered_map<std::string, QFormLayout *> group_layouts_;
    };

    // ============================================================================
    // Property Tree Widget - Hierarchical view of object properties
    // ============================================================================

    template <typename T> class PropertyTreeEditor : public QTreeWidget {
    public:
        explicit PropertyTreeEditor(T *object, QWidget *parent = nullptr);

        void setObject(T *object);
        void refresh();
        void setPropertyChangedCallback(PropertyChangedCallback callback);

    private:
        void             setupUi();
        void             populateTree();
        QTreeWidgetItem *createFieldItem(const std::string &field_name, std::type_index type);

        T                            *object_   = nullptr;
        const core::ClassMetadata<T> *metadata_ = nullptr;
        PropertyChangedCallback       on_change_callback_;
    };

    // ============================================================================
    // Multi-Object Property Editor - Edit multiple objects simultaneously
    // ============================================================================

    template <typename T> class MultiObjectPropertyEditor : public QWidget {
    public:
        explicit MultiObjectPropertyEditor(QWidget *parent = nullptr);

        void setObjects(const std::vector<T *> &objects);
        void addObject(T *object);
        void removeObject(T *object);
        void clear();
        void refresh();

        void setPropertyChangedCallback(PropertyChangedCallback callback);

    private:
        void setupUi();
        void updateWidgets();
        bool allObjectsHaveSameValue(const std::string &field_name) const;

        std::vector<T *>                           objects_;
        const core::ClassMetadata<T>              *metadata_    = nullptr;
        QFormLayout                               *form_layout_ = nullptr;
        PropertyChangedCallback                    on_change_callback_;
        std::unordered_map<std::string, QWidget *> field_widgets_;
    };

    // ============================================================================
    // Method Invoker Widget - Button panel for invoking methods
    // ============================================================================

    template <typename T> class MethodInvoker : public QWidget {
    public:
        explicit MethodInvoker(T *object, QWidget *parent = nullptr);

        void setObject(T *object);

        /**
         * @brief Filter which methods to show
         */
        void setMethodFilter(std::function<bool(const std::string &)> filter);

    private:
        void     setupUi();
        void     createMethodButtons();
        QWidget *createMethodWidget(const std::string &method_name);

        T                                       *object_   = nullptr;
        const core::ClassMetadata<T>            *metadata_ = nullptr;
        QVBoxLayout                             *layout_   = nullptr;
        std::function<bool(const std::string &)> method_filter_;
    };

    // ============================================================================
    // Object Inspector - Combined property editor + method invoker
    // ============================================================================

    template <typename T> class ObjectInspector : public QWidget {
    public:
        explicit ObjectInspector(T *object, QWidget *parent = nullptr);

        void setObject(T *object);
        void refresh();

        PropertyEditor<T> *propertyEditor() { return property_editor_; }
        MethodInvoker<T>  *methodInvoker() { return method_invoker_; }

    private:
        void setupUi();

        T                 *object_          = nullptr;
        QTabWidget        *tab_widget_      = nullptr;
        PropertyEditor<T> *property_editor_ = nullptr;
        MethodInvoker<T>  *method_invoker_  = nullptr;
    };

} // namespace rosetta::qt

#include "inline/qt_property_editor.hxx"