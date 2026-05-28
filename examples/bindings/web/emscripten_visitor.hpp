// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Emscripten/embind backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::EmscriptenVisitor<T> — three visitor methods emitting
//     emscripten::class_<T> registration calls
//   - rosetta::bind_emscripten<T>(class_name) — entry point: builds the
//     class_ handle, registers a default ctor, runs the walk
//
// All lambdas are non-capturing (Fld / Fn are NTTPs of the enclosing
// template, and per-field annotations live as constexpr locals usable
// as constant expressions) so embind can take them as function-pointer
// types via `+[]`.

#pragma once

#include <emscripten/bind.h>
#include <experimental/meta>
#include <rosetta/walk.h>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace rosetta {

    template <typename T> struct EmscriptenVisitor {
        emscripten::class_<T> &cls;

        template <std::meta::info Fld, auto... Anns> void field(const char * name) {
            using F = [:std::meta::type_of(Fld):];

            if constexpr (ann::has<readonly>(Anns...)) {
                // Pair the getter with a throwing setter so JS-side assignment
                // surfaces a clear error (embind silently no-ops a getter-only
                // accessor in non-strict mode).
                cls.property(
                    name, +[](const T &s) -> F { return s.[:Fld:]; },
                    +[](T &, F) {
                        constexpr auto fname =
                            std::define_static_string(std::meta::identifier_of(Fld));
                        throw std::runtime_error(std::string(fname) + " is read-only");
                    });
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                cls.property(
                    name, +[](const T &s) -> F { return s.[:Fld:]; },
                    +[](T &s, F v) {
                        constexpr auto fname =
                            std::define_static_string(std::meta::identifier_of(Fld));
                        double d = static_cast<double>(v);
                        if (d < r.min || d > r.max) {
                            throw std::runtime_error(std::string(fname) + " out of range");
                        }
                        s.[:Fld:] = v;
                    });
            } else {
                cls.property(
                    name, +[](const T &s) -> F { return s.[:Fld:]; },
                    +[](T &s, F v) { s.[:Fld:] = v; });
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_instance(const char *name) {
            cls.function(name, &[:Fn:]);
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_static(const char *name) {
            cls.class_function(name, &[:Fn:]);
        }
    };

    template <typename T> void bind_emscripten(const char *class_name) {
        emscripten::class_<T> cls(class_name);
        cls.template constructor<>();
        EmscriptenVisitor<T> v{cls};
        walk<T>(v);
    }

} // namespace rosetta
