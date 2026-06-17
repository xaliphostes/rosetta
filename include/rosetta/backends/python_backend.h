// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Python (pybind11) generation backend — declaration. Part of the generate
// pipeline (included by inline/generate.hxx after the shared render helpers);
// the emit() implementation and any source templates live in
// inline/python_backend.hxx. Not a standalone header — it relies on Backend /
// GenContext from <rosetta/generate.h> being already visible.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct PythonBackend : Backend {
            void        emit(const GenContext &c) const override;
            // Returns the generated auto_pybind.cpp source (trampolines + bindings),
            // the same text emit() writes — exposed so it can be inspected/tested.
            std::string render(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/python_backend.hxx"
