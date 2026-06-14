// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// OpenAPI 3.1 generation backend — declaration. Part of the generate pipeline
// (included by inline/generate.hxx after the shared render helpers); the emit()
// implementation and any source templates live in inline/openapi_backend.hxx.
// Not a standalone header — it relies on Backend / GenContext from
// <rosetta/generate.h> being already visible.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct OpenApiBackend : Backend {
            void emit(const GenContext &c) const override;
        };

    } // namespace gen_detail
} // namespace rosetta

#include "inline/openapi_backend.hxx"
