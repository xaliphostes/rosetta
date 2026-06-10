// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-CxxWrap demo: the binding kit lives in
// <rosetta/visitors/julia_visitor.h> and <rosetta/walk.h>. The demo type
// lives in ../person.h. This file just registers it with jlcxx — the same
// `Person` struct bound by the Python and Node examples, showing the
// visitor pattern is language-agnostic.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// (see CMakeLists.txt in this directory).

#include "../../person.h"
#include <jlcxx/jlcxx.hpp>
#include <rosetta/visitors/julia_visitor.h>

// CxxWrap.jl loads this module entry point (default symbol name
// `define_julia_module`) when Julia does `@wrapmodule(() -> libpath)`.
JLCXX_MODULE define_julia_module(jlcxx::Module &mod) {
    rosetta::bind_julia<Person>(mod, "Person");
}
