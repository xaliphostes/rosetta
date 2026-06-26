// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Java backend (thin): implements the rosetta::walk visitor concept.
//
// This is the reflective half of the Java binding — the exact sibling of
// <rosetta/visitors/csharp_visitor.h>. It fills the runtime registry in
// <rosetta/backends/java_runtime.h> (Store, registry, JSON marshalling, the
// api_* dispatch and the exported C ABI surface) by walking T's reflected
// members and storing splice-based handlers — so building the generated
// auto_java.cpp needs the C++26 reflection toolchain.
//
// Provides:
//   - rosetta::bind_java<T>(type_name)          — register a class
//   - rosetta::bind_java_enum<T>(type_name)     — register an enum's values
//   - rosetta::bind_java_function<F>(name)      — register a free function
//
// Boundary surface (same as C# / REST): scalars, bool, std::string, enums (as
// their underlying integer) and std::vector of those. Members using any other
// type are skipped — they have no faithful Java value mapping over the JSON
// boundary.
//
// The implementation lives in inline/java_visitor.hxx.

#pragma once

#include <cstdint>
#include <experimental/meta>
#include <rosetta/backends/java_runtime.h>
#include <rosetta/walk.h>
#include <type_traits>

namespace rosetta {

    // Register class T under `type_name` in the Java runtime registry. Runs the
    // reflection walk once and stores type-erased, JSON-marshalling handlers.
    template <typename T> void bind_java(const char *type_name);

    // Register enum T: its enumerators are exposed as a name -> value map.
    template <typename T> void bind_java_enum(const char *type_name);

    // Register a free function (named by its reflection F) under `name`. Skipped
    // (no registration) if any parameter / return type can't cross the JSON
    // boundary.
    template <std::meta::info F> void bind_java_function(const char *name);

} // namespace rosetta

#include "inline/java_visitor.hxx"
