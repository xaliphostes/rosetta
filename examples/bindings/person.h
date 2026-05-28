// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Shared demo type — annotated with rosetta::doc / range / readonly.
// Both examples/python/auto_pybind.cpp and examples/node/auto_napi.cpp
// bind this same struct to demonstrate that the visitor pattern is
// language-agnostic.

#pragma once

#include <rosetta/annotations.h>
#include <string>
#include <utility>

struct Person {
    [[= rosetta::doc{"the person's display name"}]]
    std::string name;

    [[ = rosetta::doc{"age in whole years"}, = rosetta::range{0.0, 150.0} ]]
    int age = 0;

    [[ = rosetta::doc{"server-assigned identifier"}, = rosetta::readonly{} ]]
    std::string id;

    Person() = default;
    Person(std::string n, int a, std::string i) : name(std::move(n)), age(a), id(std::move(i)) {}

    [[= rosetta::doc{"Returns a greeting prefixed by the given salutation."}]]
    std::string greet(const std::string &salutation) const {
        return salutation + ", " + name + "!";
    }

    void clear() {
        name.clear();
        age = 0;
    }
};
