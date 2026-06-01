// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Emscripten/embind backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::EmscriptenVisitor<T> — visitor methods emitting
//     emscripten::class_<T> registration calls, including one
//     `constructor<Params...>()` per enumerated constructor
//   - rosetta::bind_wasm<T>(class_name) — entry point: builds the
//     class_ handle, runs the walk, and registers a default ctor as a
//     fallback when the walk surfaced none
//
// embind dispatches overloaded constructors by argument count only: two
// constructors with the same arity but different parameter types cannot
// be distinguished and embind rejects the duplicate at registration.
//
// All lambdas are non-capturing (Fld / Fn are NTTPs of the enclosing
// template, and per-field annotations live as constexpr locals usable
// as constant expressions) so embind can take them as function-pointer
// types via `+[]`.
//
// The implementation lives in inline/wasm_visitor.hxx.

#pragma once

#include <emscripten/bind.h>
#include <experimental/meta>
#include <rosetta/walk.h>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace rosetta {

    template <typename T> void bind_wasm(const char *);

}

#include "inline/wasm_visitor.hxx"
