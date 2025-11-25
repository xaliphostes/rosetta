namespace rosetta::qt {

    inline void register_int_slider(int min = 0, int max = 100) {
        WidgetFactory::instance().register_widget(
            std::type_index(typeid(int)),
            [min, max](const std::string &field_name, std::function<core::Any()> getter,
                       std::function<void(const core::Any &)> setter,
                       PropertyChangedCallback                on_change) -> QWidget * {
                auto *container = new QWidget();
                auto *layout    = new QHBoxLayout(container);
                layout->setContentsMargins(0, 0, 0, 0);

                auto *slider  = new QSlider(Qt::Horizontal);
                auto *spinbox = new QSpinBox();

                slider->setRange(min, max);
                spinbox->setRange(min, max);

                int value = getter().as<int>();
                slider->setValue(value);
                spinbox->setValue(value);

                // Sync slider and spinbox
                QObject::connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
                QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), slider,
                                 &QSlider::setValue);

                // Update on change
                QObject::connect(slider, &QSlider::valueChanged,
                                 [setter, on_change, field_name](int val) {
                                     setter(core::Any(val));
                                     if (on_change)
                                         on_change(field_name);
                                 });

                layout->addWidget(slider, 1);
                layout->addWidget(spinbox, 0);

                return container;
            });
    }

    inline void register_int_combo(const std::vector<std::string> &options) {
        WidgetFactory::instance().register_widget(
            std::type_index(typeid(int)),
            [options](const std::string &field_name, std::function<core::Any()> getter,
                      std::function<void(const core::Any &)> setter,
                      PropertyChangedCallback                on_change) -> QWidget * {
                auto *combo = new QComboBox();
                for (const auto &opt : options) {
                    combo->addItem(QString::fromStdString(opt));
                }

                int value = getter().as<int>();
                if (value >= 0 && value < static_cast<int>(options.size())) {
                    combo->setCurrentIndex(value);
                }

                QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                 [setter, on_change, field_name](int index) {
                                     setter(core::Any(index));
                                     if (on_change)
                                         on_change(field_name);
                                 });

                return combo;
            });
    }

    inline void register_path_selector(const std::string &filter    = "All Files (*.*)",
                                       bool               directory = false) {
        WidgetFactory::instance().register_widget(
            std::type_index(typeid(std::string)),
            [filter, directory](const std::string &field_name, std::function<core::Any()> getter,
                                std::function<void(const core::Any &)> setter,
                                PropertyChangedCallback                on_change) -> QWidget * {
                auto *container = new QWidget();
                auto *layout    = new QHBoxLayout(container);
                layout->setContentsMargins(0, 0, 0, 0);

                auto *lineedit = new QLineEdit();
                auto *browse   = new QPushButton("...");
                browse->setMaximumWidth(30);

                lineedit->setText(QString::fromStdString(getter().as<std::string>()));

                QObject::connect(lineedit, &QLineEdit::textChanged,
                                 [setter, on_change, field_name](const QString &text) {
                                     setter(core::Any(text.toStdString()));
                                     if (on_change)
                                         on_change(field_name);
                                 });

                QObject::connect(browse, &QPushButton::clicked, [=]() {
                    QString path;
                    if (directory) {
                        path = QFileDialog::getExistingDirectory(container, "Select Directory",
                                                                 lineedit->text());
                    } else {
                        path =
                            QFileDialog::getOpenFileName(container, "Select File", lineedit->text(),
                                                         QString::fromStdString(filter));
                    }
                    if (!path.isEmpty()) {
                        lineedit->setText(path);
                    }
                });

                layout->addWidget(lineedit, 1);
                layout->addWidget(browse, 0);

                return container;
            });
    }

    inline void register_text_area() {
        WidgetFactory::instance().register_widget(
            std::type_index(typeid(std::string)),
            [](const std::string &field_name, std::function<core::Any()> getter,
               std::function<void(const core::Any &)> setter,
               PropertyChangedCallback                on_change) -> QWidget * {
                auto *textedit = new QTextEdit();
                textedit->setMaximumHeight(100);
                textedit->setPlainText(QString::fromStdString(getter().as<std::string>()));

                QObject::connect(textedit, &QTextEdit::textChanged, [=]() {
                    setter(core::Any(textedit->toPlainText().toStdString()));
                    if (on_change)
                        on_change(field_name);
                });

                return textedit;
            });
    }

    template <typename T>
    inline void register_widget_for_type(
        std::function<QWidget *(const std::string &, std::function<core::Any()>,
                                std::function<void(const core::Any &)>, PropertyChangedCallback)>
            creator) {
        WidgetFactory::instance().register_widget(std::type_index(typeid(T)), std::move(creator));
    }

    // ============================================================================
    // Property Editor Builder - Fluent API for configuration
    // ============================================================================

    template <typename T>
    inline PropertyEditorBuilder<T>::PropertyEditorBuilder(T *object) : object_(object) {
        editor_ = new PropertyEditor<T>(object);
    }

    /**
     * @brief Set a field to read-only
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &
    PropertyEditorBuilder<T>::readOnly(const std::string &field_name) {
        // Would need to extend PropertyEditor to support per-field read-only
        return *this;
    }

    /**
     * @brief Hide a field
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &PropertyEditorBuilder<T>::hide(const std::string &field_name) {
        hidden_fields_.push_back(field_name);
        return *this;
    }

    /**
     * @brief Group fields together
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &
    PropertyEditorBuilder<T>::group(const std::string              &group_name,
                                    const std::vector<std::string> &fields) {
        for (const auto &f : fields) {
            editor_->setFieldGroup(f, group_name);
        }
        return *this;
    }

    /**
     * @brief Set display label for a field
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &
    PropertyEditorBuilder<T>::label(const std::string &field_name,
                                    const std::string &display_label) {
        labels_[field_name] = display_label;
        return *this;
    }

    /**
     * @brief Set tooltip for a field
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &
    PropertyEditorBuilder<T>::tooltip(const std::string &field_name,
                                      const std::string &tooltip_text) {
        tooltips_[field_name] = tooltip_text;
        return *this;
    }

    /**
     * @brief Set numeric range constraint
     */
    template <typename T>
    template <typename ValueType>
    inline PropertyEditorBuilder<T> &PropertyEditorBuilder<T>::range(const std::string &field_name,
                                                                     ValueType          min_val,
                                                                     ValueType          max_val) {
        // Would need widget factory customization per field
        return *this;
    }

    /**
     * @brief Set change callback
     */
    template <typename T>
    inline PropertyEditorBuilder<T> &
    PropertyEditorBuilder<T>::onChange(PropertyChangedCallback callback) {
        editor_->setPropertyChangedCallback(std::move(callback));
        return *this;
    }

    /**
     * @brief Build and return the editor
     */
    template <typename T> inline PropertyEditor<T> *PropertyEditorBuilder<T>::build() {
        // Apply hidden fields
        for (const auto &f : hidden_fields_) {
            editor_->setFieldVisible(f, false);
        }

        // Note: labels and tooltips would require extending PropertyEditor

        return editor_;
    }

    // -------------------------------------------------

    /**
     * @brief Convenience function to create a property editor builder
     */
    template <typename T> inline PropertyEditorBuilder<T> makePropertyEditor(T *object) {
        return PropertyEditorBuilder<T>(object);
    }

    template <typename T> inline PropertyEditorBuilder<T> makePropertyEditor(T &object) {
        return PropertyEditorBuilder<T>(&object);
    }

    // ============================================================================
    // Property Binding - Two-way data binding between objects
    // ============================================================================

    /**
     * @brief Bind a property between two objects
     *
     * When source.field changes, target.field is updated automatically
     */
    template <typename SourceType, typename TargetType>
    inline PropertyBinding<SourceType, TargetType>::PropertyBinding(SourceType        *source,
                                                                    const std::string &source_field,
                                                                    TargetType        *target,
                                                                    const std::string &target_field,
                                                                    bool bidirectional)
        : source_(source), target_(target), source_field_(source_field),
          target_field_(target_field), bidirectional_(bidirectional) {
        source_meta_ = &core::Registry::instance().get<SourceType>();
        target_meta_ = &core::Registry::instance().get<TargetType>();
    }

    /**
     * @brief Sync target from source
     */
    template <typename SourceType, typename TargetType>
    inline void PropertyBinding<SourceType, TargetType>::syncToTarget() {
        core::Any value = source_meta_->get_field(*source_, source_field_);
        target_meta_->set_field(*target_, target_field_, value);
    }

    /**
     * @brief Sync source from target (if bidirectional)
     */
    template <typename SourceType, typename TargetType>
    inline void PropertyBinding<SourceType, TargetType>::syncToSource() {
        if (bidirectional_) {
            core::Any value = target_meta_->get_field(*target_, target_field_);
            source_meta_->set_field(*source_, source_field_, value);
        }
    }

} // namespace rosetta::qt
