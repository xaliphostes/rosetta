// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// WebAssembly (emscripten/embind) generation backend — declaration. Part of
// the generate pipeline (included by inline/generate.hxx after the shared
// render helpers); the emit() implementation and any source templates live in
// inline/wasm_backend.hxx. Not a standalone header — it relies on Backend /
// GenContext from <rosetta/generate.h> being already visible.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct WasmBackend : Backend {
            void emit(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/wasm_backend.hxx"
