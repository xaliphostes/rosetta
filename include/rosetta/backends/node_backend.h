// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Node (N-API) generation backend — declaration. Part of the generate pipeline
// (included by inline/generate.hxx after the shared render helpers); the emit()
// implementation and any source templates live in inline/node_backend.hxx. Not
// a standalone header — it relies on Backend / GenContext from
// <rosetta/generate.h> being already visible.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct NodeBackend : Backend {
            void        emit(const GenContext &c) const override;
            // Returns the generated auto_napi.cpp source (trampolines + bindings),
            // the same text emit() writes — exposed so it can be inspected/tested.
            std::string render(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/node_backend.hxx"
