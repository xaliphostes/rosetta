// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// CxxWrap (jlcxx) backend: implements the rosetta::walk visitor concept.
//
// Julia's idiomatic C++ interop library is CxxWrap.jl, whose C++ side is
// jlcxx — the same role pybind11 plays for Python and N-API for Node. A
// binding is a `JLCXX_MODULE define_module(jlcxx::Module &mod)` function
// that registers types and functions; CxxWrap.jl loads the resulting
// shared library from Julia.
//
// Provides:
//   - rosetta::JuliaVisitor<T> — visitor methods emitting jlcxx::Module /
//     jlcxx::TypeWrapper calls. Fields become a getter `name(obj)` plus a
//     mutating setter `name!(obj, v)` (Julia's `!` convention); read-only
//     fields get no setter; ranged fields validate on assignment. Instance
//     methods become `name(obj, args...)`, static methods module-level
//     functions, and each constructor a `.constructor<...>()`.
//   - rosetta::bind_julia<T>(module, name) — entry point: registers the
//     type, runs the walk, and adds a default ctor as a fallback.
//   - rosetta::bind_julia_enum<T>(module, name) — register an enum as a
//     jlcxx bits type with its enumerators as module constants.
//   - rosetta::bind_julia_function<F>(module, name) — bind a free function.
//
// Members using a type jlcxx cannot marshal (e.g. std::function params)
// are skipped, not fatal — matching the Python/Node visitors.
//
// The implementation lives in inline/julia_visitor.hxx.

#pragma once

#include <experimental/meta>
#include <functional>
#include <jlcxx/jlcxx.hpp>
#include <rosetta/walk.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// NOTE: jlcxx's std::vector marshaling lives in <jlcxx/stl.hpp>, which today
// does not compile against the clang-p2996 fork's -fexperimental-library
// (it uses std::ranges::fill / lower_bound / binary_search, not yet provided).
// Until the fork's libc++ catches up, vector members are skipped by the walk
// (see julia_supported in the .hxx) rather than bound — scalars, strings,
// enums, and user class types work. Add `#include <jlcxx/stl.hpp>` and flip
// the vector case back on once the toolchain supports it.

namespace rosetta {

    template <typename T> void bind_julia(jlcxx::Module &, const char *);

    // Register an enum type as a jlcxx bits type; each enumerator is exposed
    // as a module-level constant (`Color.Red`-style access via Julia's
    // CppEnum). T must be an enumeration type.
    template <typename T> void bind_julia_enum(jlcxx::Module &, const char *);

    // Wrap a free function (named by its reflection F) as a Julia function
    // that marshals arguments and the return value through jlcxx.
    template <std::meta::info F> void bind_julia_function(jlcxx::Module &, const char *);
}

#include "inline/julia_visitor.hxx"
