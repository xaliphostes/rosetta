#pragma once

#include <QVariant>
#include <rosetta/rosetta.h>

namespace rosetta::qt {

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
        Q_INVOKABLE void setObject(const QString &className, QObject *obj);

        /**
         * @brief Clear the current object
         */
        Q_INVOKABLE void clearObject();

        /**
         * @brief Check if an object is set
         */
        bool hasObject() const;

        /**
         * @brief Get the current class name
         */
        QString className() const;

        /**
         * @brief Get list of all field names (fully type-erased)
         */
        QStringList fieldNames() const;

        /**
         * @brief Get list of all method names (fully type-erased)
         */
        QStringList methodNames() const;

        /**
         * @brief Get a field value as QVariant (fully type-erased)
         */
        Q_INVOKABLE QVariant getField(const QString &fieldName) const;

        /**
         * @brief Set a field value from QVariant (fully type-erased)
         */
        Q_INVOKABLE bool setField(const QString &fieldName, const QVariant &value);

        /**
         * @brief Get the type of a field as string (fully type-erased)
         */
        Q_INVOKABLE QString getFieldType(const QString &fieldName) const;

        /**
         * @brief Get field metadata as a JS object
         * Returns: { name: "x", type: "float", value: 1.5 }
         */
        Q_INVOKABLE QVariantMap getFieldInfo(const QString &fieldName) const;

        /**
         * @brief Get all fields with their info
         * Returns array of { name, type, value }
         */
        Q_INVOKABLE QVariantList getAllFieldsInfo() const;

        /**
         * @brief Invoke a method with arguments (fully type-erased)
         * @param methodName Name of the method
         * @param args QVariantList of arguments
         * @return Return value as QVariant
         */
        Q_INVOKABLE QVariant invokeMethod(const QString &methodName, const QVariantList &args);

        /**
         * @brief Get method signature info
         * Returns: { name: "method", returnType: "void", argTypes: ["float", "float"], arity: 2 }
         */
        Q_INVOKABLE QVariantMap getMethodInfo(const QString &methodName) const;

        /**
         * @brief Get all methods with their info
         * Returns array of { name, returnType, argTypes, arity }
         */
        Q_INVOKABLE QVariantList getAllMethodsInfo() const;

        /**
         * @brief Check if a field exists
         */
        Q_INVOKABLE bool hasField(const QString &fieldName) const;

        /**
         * @brief Check if a method exists
         */
        Q_INVOKABLE bool hasMethod(const QString &methodName) const;

        /**
         * @brief Get class inheritance info
         * Returns: { isAbstract, isPolymorphic, baseCount }
         */
        Q_INVOKABLE QVariantMap getClassInfo() const;

        /**
         * @brief Get list of all registered class names
         */
        Q_INVOKABLE static QStringList registeredClasses();

        /**
         * @brief Check if a class is registered
         */
        Q_INVOKABLE static bool isClassRegistered(const QString &className);

    signals:
        void objectChanged();
        void fieldChanged(const QString &fieldName);

    private:
        std::string                           class_name_;
        QObject                              *current_object_ = nullptr;
        void                                 *object_ptr_     = nullptr;
        const core::Registry::MetadataHolder *holder_         = nullptr;
    };

} // namespace rosetta::qt

#include "inline/qml_bridge.hxx"