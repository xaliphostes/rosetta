// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Pybind11 backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::PybindVisitor<T> — three visitor methods emitting pybind11 calls
//   - rosetta::bind_pybind<T>(module, py_name) — entry point: declares the
//     class, registers a default ctor, runs the walk

#pragma once

#include <experimental/meta>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rosetta/walk.h>
#include <string>
#include <type_traits>

namespace py = pybind11;

namespace rosetta {

    template <typename T> struct PybindVisitor {
        py::class_<T> &cls;

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F                      = [:std::meta::type_of(Fld):];
            constexpr auto        dann   = ann::get_or<doc>(doc{""}, Anns...);
            constexpr const char *docstr = dann.text;

            if constexpr (ann::has<readonly>(Anns...)) {
                cls.def_property_readonly(name, [](const T &s) -> F { return s.[:Fld:]; }, docstr);
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                cls.def_property(
                    name, [](const T &s) -> F { return s.[:Fld:]; },
                    [name](T &s, F v) {
                        double d = static_cast<double>(v);
                        if (d < r.min || d > r.max) {
                            throw py::value_error(std::string(name) + " out of range [" +
                                                  std::to_string(r.min) + ", " +
                                                  std::to_string(r.max) + "]");
                        }
                        s.[:Fld:] = v;
                    },
                    docstr);
            } else {
                cls.def_property(
                    name, [](const T &s) -> F { return s.[:Fld:]; },
                    [](T &s, F v) { s.[:Fld:] = v; }, docstr);
            }
        }

        template <std::meta::info Fn, auto... Anns> void method_instance(const char *name) {
            constexpr auto dann = ann::get_or<doc>(doc{""}, Anns...);
            cls.def(name, &[:Fn:], dann.text);
        }

        template <std::meta::info Fn, auto... Anns> void method_static(const char *name) {
            constexpr auto dann = ann::get_or<doc>(doc{""}, Anns...);
            cls.def_static(name, &[:Fn:], dann.text);
        }
    };

    template <typename T> void bind_pybind(py::module_ &m, const char *py_name) {
        py::class_<T> cls(m, py_name);
        cls.def(py::init<>());
        PybindVisitor<T> v{cls};
        walk<T>(v);
    }

} // namespace rosetta
