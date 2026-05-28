// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Shared annotation types used by every backend (pybind11, N-API, ...).
//
// Each annotation must be a structural / NTTP-eligible type. The `doc`
// constructor routes the literal through std::meta::define_static_string
// to get a pointer with linkage (a plain literal's address is not
// NTTP-eligible — see DETAILS.md).
//
// Build flag required: -fannotation-attributes

#pragma once

#include <experimental/meta>

namespace rosetta {

    /**
     * @brief Documentation string annotation. Can be attached to types, members, or function
     * parameters.
     */
    struct doc {
        const char *text;
        consteval doc(const char *t) : text(std::define_static_string(t)) {}
        bool operator==(const doc &) const = default;
    };

    /**
     * @brief Numeric range annotation for function parameters. Can be used by backends to generate
     * validation or UI widgets.
     */
    struct range {
        double min;
        double max;
        bool   operator==(const range &) const = default;
    };

    /**
     * @brief Read-only annotation for members. Can be used by backends to generate
     * read-only UI widgets.
     */
    struct readonly {
        bool operator==(const readonly &) const = default;
    };

} // namespace rosetta
