// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-embind demo: binding kit lives in <emscripten_visitor.h> and
// <rosetta/walk.h>. The demo type lives in ../person.h. This file
// just registers it inside an EMSCRIPTEN_BINDINGS block.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "../person.h"
#include "emscripten_visitor.h"

EMSCRIPTEN_BINDINGS(reflected_person) {
    rosetta::bind_emscripten<Person>("Person");
}
