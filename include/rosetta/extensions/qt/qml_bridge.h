#pragma once

#include <QVariant>
#include <rosetta/rosetta.h>

namespace rosetta::qt {

    // ============================================================================
    // Property Editor Bridge - Connect Rosetta to QML
    // ============================================================================

    /**
     * @brief Convert Rosetta core::Any to QVariant for QML
     */
    inline QVariant anyToVariant(const core::Any &value) {
        if (!value.has_value()) {
            return QVariant();
        }

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) {
            return QVariant(value.as<int>());
        }
        if (type == std::type_index(typeid(float))) {
            return QVariant(value.as<float>());
        }
        if (type == std::type_index(typeid(double))) {
            return QVariant(value.as<double>());
        }
        if (type == std::type_index(typeid(bool))) {
            return QVariant(value.as<bool>());
        }
        if (type == std::type_index(typeid(std::string))) {
            return QVariant(QString::fromStdString(value.as<std::string>()));
        }
        if (type == std::type_index(typeid(size_t))) {
            return QVariant(static_cast<qulonglong>(value.as<size_t>()));
        }
        if (type == std::type_index(typeid(long))) {
            return QVariant(static_cast<qlonglong>(value.as<long>()));
        }

        // Vectors
        if (type == std::type_index(typeid(std::vector<double>))) {
            const auto  &vec = value.as<std::vector<double>>();
            QVariantList list;
            for (double v : vec)
                list.append(v);
            return list;
        }
        if (type == std::type_index(typeid(std::vector<int>))) {
            const auto  &vec = value.as<std::vector<int>>();
            QVariantList list;
            for (int v : vec)
                list.append(v);
            return list;
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            const auto  &vec = value.as<std::vector<std::string>>();
            QVariantList list;
            for (const auto &v : vec)
                list.append(QString::fromStdString(v));
            return list;
        }

        // Void (for method returns)
        if (type == std::type_index(typeid(void))) {
            return QVariant();
        }

        // Unknown type
        return QVariant();
    }

    /**
     * @brief Convert QVariant to Rosetta core::Any with expected type
     */
    inline core::Any variantToAny(const QVariant &variant, std::type_index expectedType) {
        // Handle primitives
        if (expectedType == std::type_index(typeid(int))) {
            return core::Any(variant.toInt());
        }
        if (expectedType == std::type_index(typeid(float))) {
            return core::Any(variant.toFloat());
        }
        if (expectedType == std::type_index(typeid(double))) {
            return core::Any(variant.toDouble());
        }
        if (expectedType == std::type_index(typeid(bool))) {
            return core::Any(variant.toBool());
        }
        if (expectedType == std::type_index(typeid(std::string))) {
            return core::Any(variant.toString().toStdString());
        }
        if (expectedType == std::type_index(typeid(size_t))) {
            return core::Any(static_cast<size_t>(variant.toULongLong()));
        }
        if (expectedType == std::type_index(typeid(long))) {
            return core::Any(static_cast<long>(variant.toLongLong()));
        }

        // Vectors
        if (expectedType == std::type_index(typeid(std::vector<double>))) {
            std::vector<double> vec;
            for (const auto &v : variant.toList())
                vec.push_back(v.toDouble());
            return core::Any(vec);
        }
        if (expectedType == std::type_index(typeid(std::vector<int>))) {
            std::vector<int> vec;
            for (const auto &v : variant.toList())
                vec.push_back(v.toInt());
            return core::Any(vec);
        }
        if (expectedType == std::type_index(typeid(std::vector<std::string>))) {
            std::vector<std::string> vec;
            for (const auto &v : variant.toList())
                vec.push_back(v.toString().toStdString());
            return core::Any(vec);
        }

        // Fallback: try to infer from QVariant type
        switch (variant.typeId()) {
        case QMetaType::Int:
            return core::Any(variant.toInt());
        case QMetaType::Double:
            return core::Any(variant.toDouble());
        case QMetaType::Float:
            return core::Any(variant.toFloat());
        case QMetaType::Bool:
            return core::Any(variant.toBool());
        case QMetaType::QString:
            return core::Any(variant.toString().toStdString());
        default:
            break;
        }

        return core::Any();
    }

    /**
     * @brief Get a readable type name for QML display
     */
    inline QString typeIndexToString(std::type_index type) {
        if (type == std::type_index(typeid(int)))
            return "int";
        if (type == std::type_index(typeid(float)))
            return "float";
        if (type == std::type_index(typeid(double)))
            return "double";
        if (type == std::type_index(typeid(bool)))
            return "bool";
        if (type == std::type_index(typeid(std::string)))
            return "string";
        if (type == std::type_index(typeid(size_t)))
            return "size_t";
        if (type == std::type_index(typeid(long)))
            return "long";
        if (type == std::type_index(typeid(void)))
            return "void";
        if (type == std::type_index(typeid(std::vector<double>)))
            return "vector<double>";
        if (type == std::type_index(typeid(std::vector<int>)))
            return "vector<int>";
        if (type == std::type_index(typeid(std::vector<std::string>)))
            return "vector<string>";
        return QString::fromStdString(core::get_readable_type_name(type));
    }

    /**
     * @brief Bridge class to expose Rosetta property editing to QML.
     *
     * A QML-accessible bridge that lets QML code dynamically read/write properties and invoke
     * methods on C++ objects using Rosetta's introspection - without needing to know the specific
     * type at compile time.
     *
     *
     * This bridge allows QML to dynamically:
     * - List all fields and methods of a Rosetta-registered class
     * - Get/set field values without knowing types at compile time
     * - Invoke methods with automatic type conversion
     * - Get field metadata (types, readonly status)
     *
     * Usage in QML:
     * @code
     * QmlBridge {
     *     id: bridge
     * }
     *
     * // Set any Rosetta-registered object
     * bridge.setObject("Primitive3D", myObject)
     *
     * // Get field names and iterate
     * for (var name of bridge.fieldNames) {
     *     var value = bridge.getField(name)
     *     var type = bridge.getFieldType(name)
     * }
     *
     * // Set a field
     * bridge.setField("positionX", 5.0)
     *
     * // Invoke a method
     * bridge.invokeMethod("resetTransform", [])
     * @endcode
     */
    class QmlBridge : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString className READ className NOTIFY objectChanged)
        Q_PROPERTY(QStringList fieldNames READ fieldNames NOTIFY objectChanged)
        Q_PROPERTY(QStringList methodNames READ methodNames NOTIFY objectChanged)
        Q_PROPERTY(bool hasObject READ hasObject NOTIFY objectChanged)

    public:
        explicit QmlBridge(QObject *parent = nullptr) : QObject(parent) {}

        /**
         * @brief Set the object to edit by class name
         * @param className Rosetta-registered class name
         * @param obj Pointer to the object (must be the correct type!)
         */
        Q_INVOKABLE void setObject(const QString &className, QObject *obj) {
            class_name_     = className.toStdString();
            current_object_ = obj;
            object_ptr_     = static_cast<void *>(obj);

            // Get metadata holder for this class
            holder_ = core::Registry::instance().get_by_name(class_name_);

            emit objectChanged();
        }

        /**
         * @brief Clear the current object
         */
        Q_INVOKABLE void clearObject() {
            class_name_.clear();
            current_object_ = nullptr;
            object_ptr_     = nullptr;
            holder_         = nullptr;
            emit objectChanged();
        }

        /**
         * @brief Check if an object is set
         */
        bool hasObject() const { return current_object_ != nullptr && holder_ != nullptr; }

        /**
         * @brief Get the current class name
         */
        QString className() const { return QString::fromStdString(class_name_); }

        /**
         * @brief Get list of all field names (fully type-erased)
         */
        QStringList fieldNames() const {
            QStringList names;
            if (!holder_)
                return names;

            for (const auto &f : holder_->get_fields()) {
                names.append(QString::fromStdString(f));
            }
            return names;
        }

        /**
         * @brief Get list of all method names (fully type-erased)
         */
        QStringList methodNames() const {
            QStringList names;
            if (!holder_)
                return names;

            for (const auto &m : holder_->get_methods()) {
                names.append(QString::fromStdString(m));
            }
            return names;
        }

        /**
         * @brief Get a field value as QVariant (fully type-erased)
         */
        Q_INVOKABLE QVariant getField(const QString &fieldName) const {
            if (!hasObject())
                return QVariant();

            try {
                core::Any value = holder_->get_field_void_ptr(object_ptr_, fieldName.toStdString());
                return anyToVariant(value);
            } catch (const std::exception &e) {
                qWarning() << "QmlBridge::getField failed:" << e.what();
                return QVariant();
            }
        }

        /**
         * @brief Set a field value from QVariant (fully type-erased)
         */
        Q_INVOKABLE bool setField(const QString &fieldName, const QVariant &value) {
            if (!hasObject())
                return false;

            try {
                std::string fname = fieldName.toStdString();

                // Get expected type from holder
                std::type_index fieldType = holder_->get_field_type(fname);

                core::Any anyValue = variantToAny(value, fieldType);
                holder_->set_field_void_ptr(object_ptr_, fname, anyValue);

                emit fieldChanged(fieldName);
                return true;
            } catch (const std::exception &e) {
                qWarning() << "QmlBridge::setField failed:" << e.what();
                return false;
            }
        }

        /**
         * @brief Get the type of a field as string (fully type-erased)
         */
        Q_INVOKABLE QString getFieldType(const QString &fieldName) const {
            if (!holder_)
                return "unknown";

            try {
                std::type_index type = holder_->get_field_type(fieldName.toStdString());
                return typeIndexToString(type);
            } catch (...) {
                return "unknown";
            }
        }

        /**
         * @brief Get field metadata as a JS object
         * Returns: { name: "x", type: "float", value: 1.5 }
         */
        Q_INVOKABLE QVariantMap getFieldInfo(const QString &fieldName) const {
            QVariantMap info;
            info["name"]  = fieldName;
            info["type"]  = getFieldType(fieldName);
            info["value"] = getField(fieldName);
            return info;
        }

        /**
         * @brief Get all fields with their info
         * Returns array of { name, type, value }
         */
        Q_INVOKABLE QVariantList getAllFieldsInfo() const {
            QVariantList list;
            for (const auto &name : fieldNames()) {
                list.append(getFieldInfo(name));
            }
            return list;
        }

        /**
         * @brief Invoke a method with arguments (fully type-erased)
         * @param methodName Name of the method
         * @param args QVariantList of arguments
         * @return Return value as QVariant
         */
        Q_INVOKABLE QVariant invokeMethod(const QString &methodName, const QVariantList &args) {
            if (!hasObject())
                return QVariant();

            try {
                std::string mname = methodName.toStdString();

                // Get expected arg types
                auto argTypes = holder_->get_method_arg_types(mname);

                // Convert QVariantList to vector<Any>
                std::vector<core::Any> anyArgs;
                for (int i = 0; i < args.size() && i < static_cast<int>(argTypes.size()); ++i) {
                    anyArgs.push_back(variantToAny(args[i], argTypes[i]));
                }

                // Invoke via type-erased interface
                core::Any result =
                    holder_->invoke_method_void_ptr(object_ptr_, mname, std::move(anyArgs));
                return anyToVariant(result);
            } catch (const std::exception &e) {
                qWarning() << "QmlBridge::invokeMethod failed:" << e.what();
                return QVariant();
            }
        }

        /**
         * @brief Get method signature info
         * Returns: { name: "method", returnType: "void", argTypes: ["float", "float"], arity: 2 }
         */
        Q_INVOKABLE QVariantMap getMethodInfo(const QString &methodName) const {
            QVariantMap info;
            info["name"] = methodName;

            if (holder_) {
                try {
                    std::string mname      = methodName.toStdString();
                    size_t      arity      = holder_->get_method_arity(mname);
                    auto        argTypes   = holder_->get_method_arg_types(mname);
                    auto        returnType = holder_->get_method_return_type(mname);

                    info["arity"]      = static_cast<int>(arity);
                    info["returnType"] = typeIndexToString(returnType);

                    QStringList argTypeNames;
                    for (const auto &t : argTypes) {
                        argTypeNames.append(typeIndexToString(t));
                    }
                    info["argTypes"] = argTypeNames;
                } catch (...) {
                    info["arity"]      = 0;
                    info["returnType"] = "unknown";
                    info["argTypes"]   = QStringList();
                }
            }

            return info;
        }

        /**
         * @brief Get all methods with their info
         * Returns array of { name, returnType, argTypes, arity }
         */
        Q_INVOKABLE QVariantList getAllMethodsInfo() const {
            QVariantList list;
            for (const auto &name : methodNames()) {
                list.append(getMethodInfo(name));
            }
            return list;
        }

        /**
         * @brief Check if a field exists
         */
        Q_INVOKABLE bool hasField(const QString &fieldName) const {
            if (!holder_)
                return false;
            return holder_->has_field(fieldName.toStdString());
        }

        /**
         * @brief Check if a method exists
         */
        Q_INVOKABLE bool hasMethod(const QString &methodName) const {
            if (!holder_)
                return false;
            return holder_->has_method(methodName.toStdString());
        }

        /**
         * @brief Get class inheritance info
         * Returns: { isAbstract, isPolymorphic, baseCount }
         */
        Q_INVOKABLE QVariantMap getClassInfo() const {
            QVariantMap info;
            info["className"] = className();

            if (holder_) {
                const auto &inh              = holder_->get_inheritance();
                info["isAbstract"]           = inh.is_abstract;
                info["isPolymorphic"]        = inh.is_polymorphic;
                info["hasVirtualDestructor"] = inh.has_virtual_destructor;
                info["baseCount"]            = static_cast<int>(inh.total_base_count());

                QStringList bases;
                for (const auto &b : inh.base_classes) {
                    bases.append(QString::fromStdString(b.name));
                }
                for (const auto &b : inh.virtual_bases) {
                    bases.append(QString::fromStdString(b.name) + " (virtual)");
                }
                info["bases"] = bases;
            }

            return info;
        }

        /**
         * @brief Get list of all registered class names
         */
        Q_INVOKABLE static QStringList registeredClasses() {
            QStringList names;
            for (const auto &name : core::Registry::instance().list_classes()) {
                names.append(QString::fromStdString(name));
            }
            return names;
        }

        /**
         * @brief Check if a class is registered
         */
        Q_INVOKABLE static bool isClassRegistered(const QString &className) {
            return core::Registry::instance().has_class(className.toStdString());
        }

    signals:
        void objectChanged();
        void fieldChanged(const QString &fieldName);

    private:
        std::string                     class_name_;
        QObject                        *current_object_ = nullptr;
        void                           *object_ptr_     = nullptr;
        const core::Registry::MetadataHolder *holder_         = nullptr;
    };

} // namespace rosetta::qt