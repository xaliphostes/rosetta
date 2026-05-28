// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-QML demo: a generic QtQuick inspector window driven by one
// rosetta::walk over Person. Field rows and method rows in Main.qml
// are rendered from the metadata the visitor pushes into a single
// ReflectedObject.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "../bindings/person.h"
#include "qml_visitor.h"
#include "reflected_object.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    Person          person;
    ReflectedObject reflected;
    reflected.setTypeName(QStringLiteral("Person"));
    rosetta::bind_qml<Person>(&reflected, person);
    reflected.emitReady();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("inspector"), &reflected);
    engine.loadFromModule("ReflectedQml", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
