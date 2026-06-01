// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-embind demo: binding kit lives in <rosetta/visitors/wasm_visitor.h>
// and <rosetta/walk.h>. The demo type lives in ../person.h. This file
// just registers it inside an EMSCRIPTEN_BINDINGS block.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "../person.h"
#include <rosetta/visitors/wasm_visitor.h>

EMSCRIPTEN_BINDINGS(reflected_person) {
    rosetta::bind_wasm<Person>("Person");
}
