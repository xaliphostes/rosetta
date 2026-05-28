// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Auto-docgen demo: emits Markdown documentation for the annotated
// Person struct (../bindings/person.h) via the rosetta::walk visitor
// in <rosetta/docgen.h>. No external dependencies — just libc++.
//
// Build flags: -freflection -freflection-latest -fannotation-attributes
// See CMakeLists.txt in this folder.
//
//   ./build/auto_docgen              # prints to stdout
//   ./build/auto_docgen > Person.md  # capture to a file

#include "../bindings/person.h"
#include <cstdio>
#include <rosetta/docgen.h>

int main() {
    const auto md = rosetta::generate_markdown<Person>();
    std::fputs(md.c_str(), stdout);
    return 0;
}
