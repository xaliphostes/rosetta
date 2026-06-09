// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-Qt-Widgets demo: a property/method inspector window built by
// one rosetta::walk over Person. The visitor in widget_visitor.h
// constructs QSpinBox / QLineEdit / QCheckBox / QPushButton rows
// directly — no QML, no moc on our side.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "algo.h"
#include <rosetta/visitors/qt_visitor.h>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Algo     algo;
    QWidget *inspector = rosetta::build_inspector<Algo>(algo, QStringLiteral("Algo"), false);
    inspector->resize(300, 300);
    inspector->show();

    return app.exec();
}
