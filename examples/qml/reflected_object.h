// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Backend-agnostic QObject adapter populated by QmlVisitor<T>. Exposes
// the reflected struct to QML as two list models plus generic
// getField / setField / callMethod invokables. Has no template
// parameter, so a single moc-generated meta-object covers any
// reflected type.

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <functional>

class ReflectedObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString typeName READ typeName CONSTANT)
    Q_PROPERTY(QVariantList fields READ fields NOTIFY fieldsChanged)
    Q_PROPERTY(QVariantList methods READ methods NOTIFY methodsChanged)

public:
    using Getter  = std::function<QVariant()>;
    using Setter  = std::function<QString(const QVariant &)>;
    using Invoker = std::function<QVariant(const QVariantList &)>;

    explicit ReflectedObject(QObject *parent = nullptr) : QObject(parent) {}

    QString       typeName() const { return type_name_; }
    QVariantList  fields() const { return fields_; }
    QVariantList  methods() const { return methods_; }

    void setTypeName(QString n) { type_name_ = std::move(n); }
    void appendField(QVariantMap info) { fields_.append(std::move(info)); }
    void appendMethod(QVariantMap info) { methods_.append(std::move(info)); }
    void registerGetter(const QString &n, Getter g) { getters_.insert(n, std::move(g)); }
    void registerSetter(const QString &n, Setter s) { setters_.insert(n, std::move(s)); }
    void registerInvoker(const QString &n, Invoker i) { invokers_.insert(n, std::move(i)); }

    // Call once after the walk to publish the populated lists.
    void emitReady() {
        emit fieldsChanged();
        emit methodsChanged();
    }

    Q_INVOKABLE QVariant getField(const QString &name) const {
        auto it = getters_.find(name);
        return it == getters_.end() ? QVariant{} : it.value()();
    }

    // Returns "" on success or an error message string. Emits fieldChanged
    // on success so QML can refresh dependent bindings.
    Q_INVOKABLE QString setField(const QString &name, const QVariant &v) {
        auto it = setters_.find(name);
        if (it == setters_.end())
            return QStringLiteral("no field: ") + name;
        QString err = it.value()(v);
        if (err.isEmpty())
            emit fieldChanged(name);
        return err;
    }

    Q_INVOKABLE QVariant callMethod(const QString &name, const QVariantList &args) {
        auto it = invokers_.find(name);
        if (it == invokers_.end())
            return QVariant(QStringLiteral("no method: ") + name);
        QVariant r = it.value()(args);
        emit methodCalled(name);
        return r;
    }

signals:
    void fieldsChanged();
    void methodsChanged();
    void fieldChanged(const QString &name);
    void methodCalled(const QString &name);

private:
    QString               type_name_ = QStringLiteral("Reflected");
    QVariantList          fields_;
    QVariantList          methods_;
    QHash<QString, Getter>  getters_;
    QHash<QString, Setter>  setters_;
    QHash<QString, Invoker> invokers_;
};
