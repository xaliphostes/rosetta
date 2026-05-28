// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-Qt-Widgets demo: a property/method inspector window built by
// one rosetta::walk over Person. The visitor in widget_visitor.h
// constructs QSpinBox / QLineEdit / QCheckBox / QPushButton rows
// directly — no QML, no moc on our side.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "../bindings/person.h"
#include "widget_visitor.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Person   person;
    QWidget *inspector = rosetta::build_inspector<Person>(person, QStringLiteral("Person"));
    inspector->resize(720, 560);
    inspector->show();

    return app.exec();
}
