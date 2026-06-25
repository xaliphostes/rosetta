// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// C# generation backend — declaration. Part of the generate pipeline (included
// by inline/generate.hxx after the shared render helpers); the emit()
// implementation lives in inline/csharp_backend.hxx. Not a standalone header —
// it relies on Backend / GenContext from <rosetta/generate.h> being visible.
//
// Emits, under <out_dir>/csharp/:
//   auto_csharp.cpp  — the native shim: registers each class/enum/function with
//                      the reflection runtime (<rosetta/visitors/csharp_visitor.h>)
//                      and exports the flat C ABI (rosetta_csharp_*) C# P/Invokes.
//   CMakeLists.txt   — builds that shim into a shared library.
//   <Lib>.cs         — idiomatic C# wrappers (handle-backed classes, enums,
//                      free functions) generated from the reflected IR.
//   <Lib>.csproj     — a .NET class-library project compiling <Lib>.cs.
//   README.md        — usage notes.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct CSharpBackend : Backend {
            void        emit(const GenContext &c) const override;
            // Returns the generated native shim (auto_csharp.cpp) source — the
            // same text emit() writes — so it can be inspected / tested.
            std::string render(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/csharp_backend.hxx"
