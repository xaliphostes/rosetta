// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Smoke test for the markdown doc generator in <rosetta/docgen.h>.
// Walks Person and prints its docs to stdout.

#include "../examples/person.h"
#include <cstdio>
#include <rosetta/docgen.h>

int main() {
    const auto md = rosetta::generate_markdown<Person>();
    std::fputs(md.c_str(), stdout);
    return 0;
}
