// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Markdown documentation generator. A rosetta::walk visitor that
// emits a Markdown summary of T's fields and methods, using the same
// annotations every other backend understands. No third-party deps —
// just <string>, <sstream> on top of <rosetta/walk.h>.
//
// Usage:
//   #include <rosetta/docgen.h>
//   std::cout << rosetta::generate_markdown<Person>();
//
// Output layout:
//   # <TypeName>
//
//   ## Fields
//   | Name | Type | Description |
//   | ...  | ...  | ...         |
//
//   ## Methods
//   ### `name(p1: T1, p2: T2) → R`
//   <doc>
//
// Annotation surface:
//   doc{"..."}        -> description column / method body paragraph
//   readonly          -> "_(readonly)_" tag in the description column
//   range{lo, hi}     -> "(range: lo..hi)" tag in the description column
//   combobox{{"a","b"}}-> "(choices: a , b)" tag in the description column

#pragma once

#include <experimental/meta>
#include <rosetta/walk.h>
#include <sstream>
#include <string>

namespace rosetta {

    struct MarkdownDoc {
        std::ostringstream out;
        bool               fields_header_emitted  = false;
        bool               methods_header_emitted = false;

        void ensure_fields_header();
        void ensure_methods_header();

        template <std::meta::info Fld, auto... Anns> void field(const char *name);
        template <std::meta::info Fn, auto... Anns> void method_instance(const char *name);
        template <std::meta::info Fn, auto... Anns> void method_static(const char *name);

    private:
        template <std::meta::info Fn, auto... Anns>
        void emit_method(const char *name, bool is_static);
    };

    // Explicit type name (use this when display_string_of(^^T) is ugly,
    // e.g. for template instantiations like std::vector<int>).
    template <typename T> std::string generate_markdown(const char *type_name);

    // Auto-derived heading via std::meta::identifier_of(^^T).
    template <typename T> std::string generate_markdown();

} // namespace rosetta

#include "inline/docgen.hxx"