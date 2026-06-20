// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Nanobind generation backend — declaration. Registered under the "nanobind"
// target. Emits a thin auto_nanobind.cpp that defers to the reflection-driven
// rosetta::bind_nanobind<T> at the target's compile time (so, like the plain
// "python" target, building the binding needs the C++26 toolchain). The
// implementation lives in inline/nanobind_backend.hxx.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct NanobindBackend : Backend {
            void        emit(const GenContext &c) const override;
            std::string render(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/nanobind_backend.hxx"
