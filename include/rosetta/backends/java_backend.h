// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Java generation backend — declaration. Part of the generate pipeline
// (included by inline/generate.hxx after the shared render helpers); the emit()
// implementation lives in inline/java_backend.hxx. Not a standalone header — it
// relies on Backend / GenContext from <rosetta/generate.h> being visible.
//
// The exact sibling of the C# backend, but the wrapper side speaks Java via the
// Foreign Function & Memory API (java.lang.foreign, JEP 454 — stable in JDK 22)
// instead of P/Invoke, and (de)serialises with Jackson instead of
// System.Text.Json.
//
// Emits, under <out_dir>/java/:
//   auto_java.cpp     — the native shim: registers each class/enum/function with
//                       the reflection runtime (<rosetta/visitors/java_visitor.h>)
//                       and exports the flat C ABI (rosetta_java_*) the FFM layer
//                       downcalls into.
//   CMakeLists.txt    — builds that shim into a shared library.
//   <pkg>/Native.java — the FFM downcall handles for the C ABI.
//   <pkg>/Rt.java     — JSON marshalling + envelope unwrapping (Jackson).
//   <pkg>/<Class>.java — one idiomatic handle-backed wrapper per class.
//   <pkg>/<Enum>.java  — one Java enum per enumeration (int-valued, @JsonValue).
//   <pkg>/Functions.java — a static facade for the free functions, if any.
//   pom.xml           — a Maven build pulling in Jackson.
//   README.md         — usage notes.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct JavaBackend : Backend {
            void        emit(const GenContext &c) const override;
            // Returns the generated native shim (auto_java.cpp) source — the same
            // text emit() writes — so it can be inspected / tested.
            std::string render(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/java_backend.hxx"
