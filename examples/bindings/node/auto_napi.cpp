// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-N-API demo: the binding kit lives in <rosetta/visitors/node_visitor.h>
// and <rosetta/walk.h>. The demo type lives in ../person.h. This file
// just registers it as the Node module entry point.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.

#include "../../person.h"
#include <rosetta/visitors/node_visitor.h>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("Person", rosetta::bind_napi<Person>(env, "Person"));
    return exports;
}

NODE_API_MODULE(reflected_person, Init)
