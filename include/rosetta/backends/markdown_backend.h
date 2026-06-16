// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Markdown documentation backend — declaration. Part of the generate pipeline
// (included by inline/generate.hxx after the shared render helpers); the emit()
// implementation and any source templates live in inline/markdown_backend.hxx.
// Not a standalone header — it relies on Backend / GenContext from
// <rosetta/generate.h> being already visible.

#pragma once

namespace rosetta {
    namespace gen_detail {

        struct MarkdownBackend : Backend {
            void        emit(const GenContext &c) const override;   // writes <out>/markdown/<lib>.md
            std::string render(const GenContext &c) const override; // the same document, as a string
        };

    } // namespace gen_detail

    /// Reflection-driven Markdown reference for one or more types, as a string
    /// (no files written). The pipeline-free entry point — pass a class/enum
    /// pack; `lib` is the document title.
    template <typename... Ts> std::string to_markdown(std::string lib = "doc");

} // namespace rosetta

#include "inline/markdown_backend.hxx"
