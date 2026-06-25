// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// C# backend (thin): implements the rosetta::walk visitor concept.
//
// This is the reflective half of the C# binding. It fills the runtime registry
// in <rosetta/backends/csharp_runtime.h> (Store, registry, JSON marshalling, the
// api_* dispatch and the exported C ABI surface) by walking T's reflected
// members and storing splice-based handlers — so building the generated
// auto_csharp.cpp needs the C++26 reflection toolchain. The reflection-free
// "csharp-expanded" backend populates the *same* runtime from explicit
// member-pointer dispatch instead, and builds with a stock compiler.
//
// Provides:
//   - rosetta::bind_csharp<T>(type_name)          — register a class
//   - rosetta::bind_csharp_enum<T>(type_name)     — register an enum's values
//   - rosetta::bind_csharp_function<F>(name)      — register a free function
//
// Boundary surface (same as REST): scalars, bool, std::string, enums (as their
// underlying integer) and std::vector of those. Members using any other type are
// skipped — they have no faithful C# value mapping.
//
// The implementation lives in inline/csharp_visitor.hxx.

#pragma once

#include <cstdint>
#include <experimental/meta>
#include <rosetta/backends/csharp_runtime.h>
#include <rosetta/walk.h>
#include <type_traits>

namespace rosetta {

    // Register class T under `type_name` in the C# runtime registry. Runs the
    // reflection walk once and stores type-erased, JSON-marshalling handlers.
    template <typename T> void bind_csharp(const char *type_name);

    // Register enum T: its enumerators are exposed as a name -> value map.
    template <typename T> void bind_csharp_enum(const char *type_name);

    // Register a free function (named by its reflection F) under `name`.
    // Skipped (no registration) if any parameter / return type can't cross the
    // JSON boundary.
    template <std::meta::info F> void bind_csharp_function(const char *name);

} // namespace rosetta

#include "inline/csharp_visitor.hxx"
