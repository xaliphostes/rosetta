// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-pybind11 demo: the binding kit lives in <rosetta/visitors/python_visitor.h>
// and <rosetta/walk.h>. The demo type lives in ../person.h. This file
// just registers it with pybind11.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// (see CMakeLists.txt in this directory).

#include "../../person.h"
#include <rosetta/visitors/python_visitor.h>

PYBIND11_MODULE(reflected_person, m) {
    m.doc() = "Bindings generated automatically by C++26 reflection.";
    rosetta::bind_pybind<Person>(m, "Person");
}
