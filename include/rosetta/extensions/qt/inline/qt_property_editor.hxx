// ============================================================================
// Qt6 Automatic Property Editor - Implementation
// ============================================================================
#pragma once

#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QToolButton>

namespace rosetta::qt {

    // ============================================================================
    // WidgetFactory Implementation
    // ============================================================================

    inline WidgetFactory &WidgetFactory::instance() {
        static WidgetFactory factory;
        return factory;
    }

    inline WidgetFactory::WidgetFactory() {
        register_default_widgets();
    }

    inline void WidgetFactory::register_widget(std::type_index type, WidgetCreator creator) {
        creators_[type] = std::move(creator);
    }

    inline bool WidgetFactory::has_widget(std::type_index type) const {
        return creators_.find(type) != creators_.end();
    }

    inline QWidget *WidgetFactory::create_widget(std::type_index                        type,
                                                 const std::string                     &field_name,
                                                 std::function<core::Any()>             getter,
                                                 std::function<void(const core::Any &)> setter,
                                                 PropertyChangedCallback                on_change) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(field_name, getter, setter, on_change);
        }

        // Fallback: create a read-only label showing type name
        auto *label = new QLabel(QString::fromStdString("<unsupported type>"));
        label->setStyleSheet("color: gray; font-style: italic;");
        return label;
    }

    inline void WidgetFactory::register_default_widgets() {
        // ----------------------------------------------------------------
        // Integer types
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(int))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QSpinBox();
            spinbox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
            spinbox->setValue(getter().as<int>());

            QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                             [setter, on_change, field_name](int value) {
                                 setter(core::Any(value));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        creators_[std::type_index(typeid(unsigned int))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QSpinBox();
            spinbox->setRange(0, std::numeric_limits<int>::max());
            spinbox->setValue(static_cast<int>(getter().as<unsigned int>()));

            QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                             [setter, on_change, field_name](int value) {
                                 setter(core::Any(static_cast<unsigned int>(value)));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        creators_[std::type_index(typeid(long))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QSpinBox();
            spinbox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
            spinbox->setValue(static_cast<int>(getter().as<long>()));

            QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                             [setter, on_change, field_name](int value) {
                                 setter(core::Any(static_cast<long>(value)));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        creators_[std::type_index(typeid(size_t))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QSpinBox();
            spinbox->setRange(0, std::numeric_limits<int>::max());
            spinbox->setValue(static_cast<int>(getter().as<size_t>()));

            QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                             [setter, on_change, field_name](int value) {
                                 setter(core::Any(static_cast<size_t>(value)));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        // ----------------------------------------------------------------
        // Floating point types
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(double))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QDoubleSpinBox();
            spinbox->setRange(-1e12, 1e12);
            spinbox->setDecimals(6);
            spinbox->setSingleStep(0.1);
            spinbox->setValue(getter().as<double>());

            QObject::connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                             [setter, on_change, field_name](double value) {
                                 setter(core::Any(value));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        creators_[std::type_index(typeid(float))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *spinbox = new QDoubleSpinBox();
            spinbox->setRange(-1e12, 1e12);
            spinbox->setDecimals(4);
            spinbox->setSingleStep(0.1);
            spinbox->setValue(static_cast<double>(getter().as<float>()));

            QObject::connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                             [setter, on_change, field_name](double value) {
                                 setter(core::Any(static_cast<float>(value)));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return spinbox;
        };

        // ----------------------------------------------------------------
        // Boolean
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(bool))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *checkbox = new QCheckBox();
            checkbox->setChecked(getter().as<bool>());

            QObject::connect(checkbox, &QCheckBox::toggled,
                             [setter, on_change, field_name](bool checked) {
                                 setter(core::Any(checked));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return checkbox;
        };

        // ----------------------------------------------------------------
        // String
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(std::string))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *lineedit = new QLineEdit();
            lineedit->setText(QString::fromStdString(getter().as<std::string>()));

            QObject::connect(lineedit, &QLineEdit::textChanged,
                             [setter, on_change, field_name](const QString &text) {
                                 setter(core::Any(text.toStdString()));
                                 if (on_change)
                                     on_change(field_name);
                             });
            return lineedit;
        };

        // ----------------------------------------------------------------
        // Vector<double> - List widget with add/remove
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(std::vector<double>))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *container = new QWidget();
            auto *layout    = new QVBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);

            auto *list_widget = new QListWidget();
            list_widget->setMaximumHeight(100);

            // Populate list
            auto refresh_list = [list_widget, getter]() {
                list_widget->clear();
                auto vec = getter().as<std::vector<double>>();
                for (size_t i = 0; i < vec.size(); ++i) {
                    list_widget->addItem(QString::number(vec[i], 'g', 6));
                }
            };
            refresh_list();

            // Button row
            auto *btn_layout = new QHBoxLayout();
            auto *add_btn    = new QPushButton("+");
            auto *remove_btn = new QPushButton("-");
            add_btn->setMaximumWidth(30);
            remove_btn->setMaximumWidth(30);

            btn_layout->addWidget(add_btn);
            btn_layout->addWidget(remove_btn);
            btn_layout->addStretch();

            layout->addWidget(list_widget);
            layout->addLayout(btn_layout);

            // Add button
            QObject::connect(add_btn, &QPushButton::clicked, [=]() {
                bool   ok;
                double value = QInputDialog::getDouble(container, "Add Value", "Value:", 0.0, -1e12,
                                                       1e12, 6, &ok);
                if (ok) {
                    auto vec = getter().as<std::vector<double>>();
                    vec.push_back(value);
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            // Remove button
            QObject::connect(remove_btn, &QPushButton::clicked, [=]() {
                int row = list_widget->currentRow();
                if (row >= 0) {
                    auto vec = getter().as<std::vector<double>>();
                    vec.erase(vec.begin() + row);
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            // Double-click to edit
            QObject::connect(
                list_widget, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item) {
                    int    row = list_widget->row(item);
                    auto   vec = getter().as<std::vector<double>>();
                    bool   ok;
                    double value = QInputDialog::getDouble(container, "Edit Value",
                                                           "Value:", vec[row], -1e12, 1e12, 6, &ok);
                    if (ok) {
                        vec[row] = value;
                        setter(core::Any(vec));
                        refresh_list();
                        if (on_change)
                            on_change(field_name);
                    }
                });

            return container;
        };

        // ----------------------------------------------------------------
        // Vector<int> - List widget with add/remove
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(std::vector<int>))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *container = new QWidget();
            auto *layout    = new QVBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);

            auto *list_widget = new QListWidget();
            list_widget->setMaximumHeight(100);

            auto refresh_list = [list_widget, getter]() {
                list_widget->clear();
                auto vec = getter().as<std::vector<int>>();
                for (size_t i = 0; i < vec.size(); ++i) {
                    list_widget->addItem(QString::number(vec[i]));
                }
            };
            refresh_list();

            auto *btn_layout = new QHBoxLayout();
            auto *add_btn    = new QPushButton("+");
            auto *remove_btn = new QPushButton("-");
            add_btn->setMaximumWidth(30);
            remove_btn->setMaximumWidth(30);

            btn_layout->addWidget(add_btn);
            btn_layout->addWidget(remove_btn);
            btn_layout->addStretch();

            layout->addWidget(list_widget);
            layout->addLayout(btn_layout);

            QObject::connect(add_btn, &QPushButton::clicked, [=]() {
                bool ok;
                int  value = QInputDialog::getInt(container, "Add Value", "Value:", 0,
                                                  std::numeric_limits<int>::min(),
                                                  std::numeric_limits<int>::max(), 1, &ok);
                if (ok) {
                    auto vec = getter().as<std::vector<int>>();
                    vec.push_back(value);
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            QObject::connect(remove_btn, &QPushButton::clicked, [=]() {
                int row = list_widget->currentRow();
                if (row >= 0) {
                    auto vec = getter().as<std::vector<int>>();
                    vec.erase(vec.begin() + row);
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            return container;
        };

        // ----------------------------------------------------------------
        // Vector<string> - List widget with add/remove
        // ----------------------------------------------------------------
        creators_[std::type_index(typeid(std::vector<std::string>))] =
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
            auto *container = new QWidget();
            auto *layout    = new QVBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);

            auto *list_widget = new QListWidget();
            list_widget->setMaximumHeight(100);

            auto refresh_list = [list_widget, getter]() {
                list_widget->clear();
                auto vec = getter().as<std::vector<std::string>>();
                for (const auto &str : vec) {
                    list_widget->addItem(QString::fromStdString(str));
                }
            };
            refresh_list();

            auto *btn_layout = new QHBoxLayout();
            auto *add_btn    = new QPushButton("+");
            auto *remove_btn = new QPushButton("-");
            add_btn->setMaximumWidth(30);
            remove_btn->setMaximumWidth(30);

            btn_layout->addWidget(add_btn);
            btn_layout->addWidget(remove_btn);
            btn_layout->addStretch();

            layout->addWidget(list_widget);
            layout->addLayout(btn_layout);

            QObject::connect(add_btn, &QPushButton::clicked, [=]() {
                bool    ok;
                QString text = QInputDialog::getText(container, "Add Value",
                                                     "Value:", QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {
                    auto vec = getter().as<std::vector<std::string>>();
                    vec.push_back(text.toStdString());
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            QObject::connect(remove_btn, &QPushButton::clicked, [=]() {
                int row = list_widget->currentRow();
                if (row >= 0) {
                    auto vec = getter().as<std::vector<std::string>>();
                    vec.erase(vec.begin() + row);
                    setter(core::Any(vec));
                    refresh_list();
                    if (on_change)
                        on_change(field_name);
                }
            });

            return container;
        };
    }

    // ============================================================================
    // PropertyEditor Implementation
    // ============================================================================

    template <typename T>
    PropertyEditor<T>::PropertyEditor(T *object, QWidget *parent)
        : QWidget(parent), object_(object) {
        metadata_ = &core::Registry::instance().get<T>();
        setupUi();
    }

    template <typename T>
    PropertyEditor<T>::PropertyEditor(T &object, QWidget *parent)
        : PropertyEditor(&object, parent) {
    }

    template <typename T> void PropertyEditor<T>::setObject(T *object) {
        object_ = object;
        refresh();
    }

    template <typename T> void PropertyEditor<T>::setObject(T &object) {
        setObject(&object);
    }

    template <typename T> void PropertyEditor<T>::setupUi() {
        main_layout_ = new QVBoxLayout(this);
        main_layout_->setContentsMargins(0, 0, 0, 0);

        // Create scroll area
        scroll_area_ = new QScrollArea();
        scroll_area_->setWidgetResizable(true);
        scroll_area_->setFrameShape(QFrame::NoFrame);

        auto *scroll_widget = new QWidget();
        form_layout_        = new QFormLayout(scroll_widget);
        form_layout_->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

        scroll_area_->setWidget(scroll_widget);
        main_layout_->addWidget(scroll_area_);

        // Create field widgets
        createFieldWidgets();
    }

    template <typename T> void PropertyEditor<T>::createFieldWidgets() {
        if (!object_ || !metadata_)
            return;

        const auto &fields  = metadata_->fields();
        auto       &factory = WidgetFactory::instance();

        for (const auto &field_name : fields) {
            std::type_index field_type = metadata_->get_field_type(field_name);

            // Create getter/setter lambdas
            auto getter = [this, field_name]() -> core::Any {
                return metadata_->get_field(*object_, field_name);
            };

            auto setter = [this, field_name](const core::Any &value) {
                metadata_->set_field(*object_, field_name, value);
            };

            // Create callback
            auto on_change = [this](const std::string &name) {
                if (on_change_callback_)
                    on_change_callback_(name);
            };

            // Create widget
            QWidget *widget =
                factory.create_widget(field_type, field_name, getter, setter, on_change);

            if (widget) {
                widget->setEnabled(!read_only_);
                field_widgets_[field_name] = widget;

                // Check if field belongs to a group
                auto group_it = field_groups_.find(field_name);
                if (group_it != field_groups_.end()) {
                    const std::string &group_name = group_it->second;

                    // Create group box if it doesn't exist
                    if (group_boxes_.find(group_name) == group_boxes_.end()) {
                        auto *group_box = new QGroupBox(QString::fromStdString(group_name));
                        group_box->setCheckable(true);
                        group_box->setChecked(true);

                        auto *group_layout         = new QFormLayout(group_box);
                        group_boxes_[group_name]   = group_box;
                        group_layouts_[group_name] = group_layout;

                        form_layout_->addRow(group_box);
                    }

                    group_layouts_[group_name]->addRow(QString::fromStdString(field_name), widget);
                } else {
                    // Add to main form
                    form_layout_->addRow(QString::fromStdString(field_name), widget);
                }
            }
        }
    }

    template <typename T> void PropertyEditor<T>::refresh() {
        if (!object_ || !metadata_)
            return;

        // Block signals during refresh
        for (auto &[field_name, widget] : field_widgets_) {
            widget->blockSignals(true);
        }

        // Update each widget
        for (const auto &field_name : metadata_->fields()) {
            auto it = field_widgets_.find(field_name);
            if (it == field_widgets_.end())
                continue;

            QWidget        *widget = it->second;
            std::type_index type   = metadata_->get_field_type(field_name);
            core::Any       value  = metadata_->get_field(*object_, field_name);

            // Update based on widget type
            if (auto *spinbox = qobject_cast<QSpinBox *>(widget)) {
                if (type == std::type_index(typeid(int))) {
                    spinbox->setValue(value.as<int>());
                } else if (type == std::type_index(typeid(unsigned int))) {
                    spinbox->setValue(static_cast<int>(value.as<unsigned int>()));
                } else if (type == std::type_index(typeid(long))) {
                    spinbox->setValue(static_cast<int>(value.as<long>()));
                } else if (type == std::type_index(typeid(size_t))) {
                    spinbox->setValue(static_cast<int>(value.as<size_t>()));
                }
            } else if (auto *dspinbox = qobject_cast<QDoubleSpinBox *>(widget)) {
                if (type == std::type_index(typeid(double))) {
                    dspinbox->setValue(value.as<double>());
                } else if (type == std::type_index(typeid(float))) {
                    dspinbox->setValue(static_cast<double>(value.as<float>()));
                }
            } else if (auto *checkbox = qobject_cast<QCheckBox *>(widget)) {
                checkbox->setChecked(value.as<bool>());
            } else if (auto *lineedit = qobject_cast<QLineEdit *>(widget)) {
                lineedit->setText(QString::fromStdString(value.as<std::string>()));
            }
        }

        // Unblock signals
        for (auto &[field_name, widget] : field_widgets_) {
            widget->blockSignals(false);
        }
    }

    template <typename T>
    void PropertyEditor<T>::setPropertyChangedCallback(PropertyChangedCallback callback) {
        on_change_callback_ = std::move(callback);
    }

    template <typename T> void PropertyEditor<T>::setReadOnly(bool readonly) {
        read_only_ = readonly;
        for (auto &[name, widget] : field_widgets_) {
            widget->setEnabled(!readonly);
        }
    }

    template <typename T>
    void PropertyEditor<T>::setFieldVisible(const std::string &field_name, bool visible) {
        auto it = field_widgets_.find(field_name);
        if (it != field_widgets_.end()) {
            it->second->setVisible(visible);

            // Also hide the label
            QWidget *label = form_layout_->labelForField(it->second);
            if (label)
                label->setVisible(visible);
        }
    }

    template <typename T>
    void PropertyEditor<T>::setFieldGroup(const std::string &field_name,
                                          const std::string &group_name) {
        field_groups_[field_name] = group_name;
    }

    // ============================================================================
    // PropertyTreeEditor Implementation
    // ============================================================================

    template <typename T>
    PropertyTreeEditor<T>::PropertyTreeEditor(T *object, QWidget *parent)
        : QTreeWidget(parent), object_(object) {
        metadata_ = &core::Registry::instance().get<T>();
        setupUi();
    }

    template <typename T> void PropertyTreeEditor<T>::setObject(T *object) {
        object_ = object;
        refresh();
    }

    template <typename T> void PropertyTreeEditor<T>::setupUi() {
        setColumnCount(2);
        setHeaderLabels({"Property", "Value"});
        setAlternatingRowColors(true);
        populateTree();
    }

    template <typename T> void PropertyTreeEditor<T>::refresh() {
        clear();
        populateTree();
    }

    template <typename T> void PropertyTreeEditor<T>::populateTree() {
        if (!object_ || !metadata_)
            return;

        const auto &fields = metadata_->fields();
        for (const auto &field_name : fields) {
            std::type_index  type = metadata_->get_field_type(field_name);
            QTreeWidgetItem *item = createFieldItem(field_name, type);
            addTopLevelItem(item);
        }
    }

    template <typename T>
    QTreeWidgetItem *PropertyTreeEditor<T>::createFieldItem(const std::string &field_name,
                                                            std::type_index    type) {
        auto *item = new QTreeWidgetItem();
        item->setText(0, QString::fromStdString(field_name));

        core::Any value = metadata_->get_field(*object_, field_name);

        // Convert to string for display
        QString value_str;
        if (type == std::type_index(typeid(int))) {
            value_str = QString::number(value.as<int>());
        } else if (type == std::type_index(typeid(double))) {
            value_str = QString::number(value.as<double>(), 'g', 6);
        } else if (type == std::type_index(typeid(float))) {
            value_str = QString::number(value.as<float>(), 'g', 4);
        } else if (type == std::type_index(typeid(bool))) {
            value_str = value.as<bool>() ? "true" : "false";
        } else if (type == std::type_index(typeid(std::string))) {
            value_str = QString::fromStdString(value.as<std::string>());
        } else {
            value_str = "<complex type>";
        }

        item->setText(1, value_str);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        return item;
    }

    template <typename T>
    void PropertyTreeEditor<T>::setPropertyChangedCallback(PropertyChangedCallback callback) {
        on_change_callback_ = std::move(callback);
    }

    // ============================================================================
    // MultiObjectPropertyEditor Implementation
    // ============================================================================

    template <typename T>
    MultiObjectPropertyEditor<T>::MultiObjectPropertyEditor(QWidget *parent) : QWidget(parent) {
        metadata_ = &core::Registry::instance().get<T>();
        setupUi();
    }

    template <typename T>
    void MultiObjectPropertyEditor<T>::setObjects(const std::vector<T *> &objects) {
        objects_ = objects;
        updateWidgets();
    }

    template <typename T> void MultiObjectPropertyEditor<T>::addObject(T *object) {
        objects_.push_back(object);
        updateWidgets();
    }

    template <typename T> void MultiObjectPropertyEditor<T>::removeObject(T *object) {
        objects_.erase(std::remove(objects_.begin(), objects_.end(), object), objects_.end());
        updateWidgets();
    }

    template <typename T> void MultiObjectPropertyEditor<T>::clear() {
        objects_.clear();
        updateWidgets();
    }

    template <typename T> void MultiObjectPropertyEditor<T>::refresh() {
        updateWidgets();
    }

    template <typename T> void MultiObjectPropertyEditor<T>::setupUi() {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        auto *scroll = new QScrollArea();
        scroll->setWidgetResizable(true);

        auto *scroll_widget = new QWidget();
        form_layout_        = new QFormLayout(scroll_widget);

        scroll->setWidget(scroll_widget);
        layout->addWidget(scroll);
    }

    template <typename T> void MultiObjectPropertyEditor<T>::updateWidgets() {
        // Clear existing widgets
        while (form_layout_->count() > 0) {
            QLayoutItem *item = form_layout_->takeAt(0);
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
        field_widgets_.clear();

        if (objects_.empty() || !metadata_)
            return;

        const auto &fields  = metadata_->fields();
        auto       &factory = WidgetFactory::instance();

        for (const auto &field_name : fields) {
            std::type_index field_type = metadata_->get_field_type(field_name);

            // Check if all objects have the same value
            bool same_value = allObjectsHaveSameValue(field_name);

            // Create getter that returns first object's value
            auto getter = [this, field_name]() -> core::Any {
                if (objects_.empty())
                    return core::Any();
                return metadata_->get_field(*objects_[0], field_name);
            };

            // Create setter that sets all objects
            auto setter = [this, field_name](const core::Any &value) {
                for (T *obj : objects_) {
                    metadata_->set_field(*obj, field_name, value);
                }
            };

            auto on_change = [this](const std::string &name) {
                if (on_change_callback_)
                    on_change_callback_(name);
            };

            QWidget *widget =
                factory.create_widget(field_type, field_name, getter, setter, on_change);

            if (widget) {
                // If values differ, show indication
                if (!same_value) {
                    widget->setStyleSheet("background-color: #ffffcc;"); // Light yellow
                    widget->setToolTip("Multiple values - editing will overwrite all");
                }

                field_widgets_[field_name] = widget;
                form_layout_->addRow(QString::fromStdString(field_name), widget);
            }
        }
    }

    template <typename T>
    bool
    MultiObjectPropertyEditor<T>::allObjectsHaveSameValue(const std::string &field_name) const {
        if (objects_.size() < 2)
            return true;

        std::type_index type        = metadata_->get_field_type(field_name);
        core::Any       first_value = metadata_->get_field(*objects_[0], field_name);

        for (size_t i = 1; i < objects_.size(); ++i) {
            core::Any other_value = metadata_->get_field(*objects_[i], field_name);

            // Compare based on type
            if (type == std::type_index(typeid(int))) {
                if (first_value.as<int>() != other_value.as<int>())
                    return false;
            } else if (type == std::type_index(typeid(double))) {
                if (first_value.as<double>() != other_value.as<double>())
                    return false;
            } else if (type == std::type_index(typeid(float))) {
                if (first_value.as<float>() != other_value.as<float>())
                    return false;
            } else if (type == std::type_index(typeid(bool))) {
                if (first_value.as<bool>() != other_value.as<bool>())
                    return false;
            } else if (type == std::type_index(typeid(std::string))) {
                if (first_value.as<std::string>() != other_value.as<std::string>())
                    return false;
            }
        }

        return true;
    }

    template <typename T>
    void
    MultiObjectPropertyEditor<T>::setPropertyChangedCallback(PropertyChangedCallback callback) {
        on_change_callback_ = std::move(callback);
    }

    // ============================================================================
    // MethodInvoker Implementation
    // ============================================================================

    template <typename T>
    MethodInvoker<T>::MethodInvoker(T *object, QWidget *parent) : MethodInvokerBase(parent), object_(object) {
        metadata_ = &core::Registry::instance().get<T>();
        setupUi();
    }

    template <typename T> void MethodInvoker<T>::setObject(T *object) {
        object_ = object;
    }

    template <typename T>
    void MethodInvoker<T>::setMethodFilter(std::function<bool(const std::string &)> filter) {
        method_filter_ = std::move(filter);
        // Rebuild UI
        while (layout_->count() > 0) {
            QLayoutItem *item = layout_->takeAt(0);
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
        createMethodButtons();
    }

    template <typename T> void MethodInvoker<T>::setupUi() {
        layout_ = new QVBoxLayout(this);
        layout_->setContentsMargins(0, 0, 0, 0);
        createMethodButtons();
    }

    template <typename T> void MethodInvoker<T>::createMethodButtons() {
        if (!metadata_)
            return;

        const auto &methods = metadata_->methods();

        for (const auto &method_name : methods) {
            // Apply filter
            if (method_filter_ && !method_filter_(method_name))
                continue;

            QWidget *method_widget = createMethodWidget(method_name);
            if (method_widget) {
                layout_->addWidget(method_widget);
            }
        }

        layout_->addStretch();
    }

    template <typename T>
    QWidget *MethodInvoker<T>::createMethodWidget(const std::string &method_name) {
        size_t arity = 0;
        try {
            arity = metadata_->get_method_arity(method_name);
        } catch (...) {
            return nullptr; // Skip methods without info
        }

        auto *container = new QWidget();
        auto *layout    = new QHBoxLayout(container);
        layout->setContentsMargins(2, 2, 2, 2);

        // Method name button
        auto *invoke_btn = new QPushButton(QString::fromStdString(method_name));

        if (arity == 0) {
            // No arguments - simple invoke
            QObject::connect(invoke_btn, &QPushButton::clicked, [this, method_name]() {
                if (!object_)
                    return;
                try {
                    metadata_->invoke_method(*object_, method_name, {});
                    // QMessageBox::information(
                    //     nullptr, "Method Invoked",
                    //     QString::fromStdString(method_name + " executed successfully"));

                    emit methodInvoked(QString::fromStdString(method_name));
                } catch (const std::exception &e) {
                    QMessageBox::warning(
                        nullptr, "Error",
                        QString::fromStdString("Failed: " + std::string(e.what())));
                }
            });
        } else {
            // Has arguments - show dialog
            const auto &arg_types = metadata_->get_method_arg_types(method_name);

            QObject::connect(invoke_btn, &QPushButton::clicked, [this, method_name, arg_types]() {
                if (!object_)
                    return;

                std::vector<core::Any> args;
                for (size_t i = 0; i < arg_types.size(); ++i) {
                    std::type_index type = arg_types[i];

                    if (type == std::type_index(typeid(int))) {
                        bool ok;
                        int  val = QInputDialog::getInt(nullptr, "Argument",
                                                        QString("Argument %1 (int):").arg(i), 0,
                                                        std::numeric_limits<int>::min(),
                                                        std::numeric_limits<int>::max(), 1, &ok);
                        if (!ok)
                            return;
                        args.push_back(core::Any(val));
                    } else if (type == std::type_index(typeid(double))) {
                        bool   ok;
                        double val = QInputDialog::getDouble(
                            nullptr, "Argument", QString("Argument %1 (double):").arg(i), 0.0,
                            -1e12, 1e12, 6, &ok);
                        if (!ok)
                            return;
                        args.push_back(core::Any(val));
                    } else if (type == std::type_index(typeid(std::string))) {
                        bool    ok;
                        QString val = QInputDialog::getText(nullptr, "Argument",
                                                            QString("Argument %1 (string):").arg(i),
                                                            QLineEdit::Normal, "", &ok);
                        if (!ok)
                            return;
                        args.push_back(core::Any(val.toStdString()));
                    } else if (type == std::type_index(typeid(bool))) {
                        QStringList items = {"false", "true"};
                        bool        ok;
                        QString     val = QInputDialog::getItem(nullptr, "Argument",
                                                                QString("Argument %1 (bool):").arg(i),
                                                                items, 0, false, &ok);
                        if (!ok)
                            return;
                        args.push_back(core::Any(val == "true"));
                    } else {
                        QMessageBox::warning(nullptr, "Unsupported",
                                             QString("Argument %1 has unsupported type").arg(i));
                        return;
                    }
                }

                try {
                    metadata_->invoke_method(*object_, method_name, args);

                    // DEBUG -------------------------------
                    // std::cerr << method_name << "(";
                    // for (size_t i = 0; i < args.size() - 1; ++i) {
                    //     std::cerr << args[i].toString() << ", ";
                    // }
                    // std::cerr << args[args.size() - 1].toString() << ")\n";
                    // -------------------------------------

                    // QMessageBox::information(
                    //     nullptr, "Method Invoked",
                    //     QString::fromStdString(method_name + " executed successfully with args"));
                    
                    emit methodInvoked(QString::fromStdString(method_name));
                } catch (const std::exception &e) {
                    QMessageBox::warning(
                        nullptr, "Error",
                        QString::fromStdString("Failed: " + std::string(e.what())));
                }
            });
        }

        layout->addWidget(invoke_btn);

        // Show signature hint
        std::string sig_hint = "(";
        try {
            const auto &arg_types = metadata_->get_method_arg_types(method_name);
            for (size_t i = 0; i < arg_types.size(); ++i) {
                if (i > 0)
                    sig_hint += ", ";
                sig_hint += core::get_readable_type_name(arg_types[i]);
            }
        } catch (...) {
        }
        sig_hint += ")";

        auto *sig_label = new QLabel(QString::fromStdString(sig_hint));
        sig_label->setStyleSheet("color: gray; font-size: 10px;");
        layout->addWidget(sig_label);
        layout->addStretch();

        return container;
    }

    // ============================================================================
    // ObjectInspector Implementation
    // ============================================================================

    template <typename T>
    ObjectInspector<T>::ObjectInspector(T *object, QWidget *parent)
        : QWidget(parent), object_(object) {
        setupUi();
    }

    template <typename T> void ObjectInspector<T>::setObject(T *object) {
        object_ = object;
        property_editor_->setObject(object);
        method_invoker_->setObject(object);
    }

    template <typename T> void ObjectInspector<T>::refresh() {
        property_editor_->refresh();
    }

    template <typename T> void ObjectInspector<T>::setupUi() {
        auto *layout = new QVBoxLayout(this);

        // Class name header
        const auto &metadata = core::Registry::instance().get<T>();
        auto       *header   = new QLabel(QString::fromStdString("Class: " + metadata.name()));
        header->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
        layout->addWidget(header);

        // Tab widget
        tab_widget_ = new QTabWidget();

        // Properties tab
        property_editor_ = new PropertyEditor<T>(object_);
        tab_widget_->addTab(property_editor_, "Properties");

        // Methods tab
        method_invoker_ = new MethodInvoker<T>(object_);
        tab_widget_->addTab(method_invoker_, "Methods");

        // Add this connection:
        connect(method_invoker_, &MethodInvoker<T>::methodInvoked, this,
                [this]() { property_editor_->refresh(); });

        layout->addWidget(tab_widget_);
    }

} // namespace rosetta::qt