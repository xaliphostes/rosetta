// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// One consteval walk over T's reflected members. Each field/method is
// passed to the visitor together with its *full annotation pack* as
// non-type template parameters; the visitor decides which annotations
// it cares about — there is no per-shape entry-point in the walker.
//
// Visitor concept — three `consteval`-template members:
//
//   v.template field          <Fld, Anns...>(name);
//   v.template method_instance<Fn,  Anns...>(name);
//   v.template method_static  <Fn,  Anns...>(name);
//
// `Fld` / `Fn` are `std::meta::info` NTTPs. `Anns...` is a pack of
// annotation values (`auto...`) — each entry can be any structural
// annotation type (rosetta::doc, rosetta::range, rosetta::readonly, or
// future kinds like widget, alias, deprecated). `name` is a
// `const char*` to static storage (via define_static_string).
//
// Helpers `rosetta::ann::has<A>(Anns...)` and
// `rosetta::ann::get_or<A>(fallback, Anns...)` let visitors query the
// pack without writing the fold/if-constexpr boilerplate themselves.

#pragma once

#include <experimental/meta>
#include <rosetta/annotations.h>
#include <type_traits>
#include <utility>

namespace rosetta {

    // -------- annotation pack helpers --------
    //
    // Both helpers take the annotation pack as runtime arguments (still
    // constant expressions because each Anns value flows through unchanged).
    // Call style at the visitor:
    //     ann::has<readonly>(Anns...)
    //     ann::get_or<doc>(doc{""}, Anns...).text
    namespace ann {

        /**
         * @brief Check if an annotation of type A is present in the pack.
         */
        template <typename A> consteval bool has(auto... anns) {
            return (std::same_as<std::remove_cvref_t<decltype(anns)>, A> || ...);
        }

        /**
         * @brief Get the first annotation of type A in the pack, or return a fallback value if absent.
         */
        template <typename A> consteval A get_or(A fallback, auto... anns) {
            A result = fallback;
            (
                [&] {
                    if constexpr (std::same_as<std::remove_cvref_t<decltype(anns)>, A>) {
                        result = anns;
                    }
                }(),
                ...);
            return result;
        }

    } // namespace ann

    // Keep regular and static methods; drop constructors/destructors and
    // the compiler-generated copy/move specials.
    consteval bool is_exportable_member_function(std::meta::info fn) {
        return std::meta::is_function(fn) && !std::meta::is_constructor(fn) &&
               !std::meta::is_destructor(fn) && !std::meta::is_special_member_function(fn);
    }

    /**
     * @brief Walk over reflected members of T and pass them to the visitor.
     */
    template <typename T, typename Visitor> void walk(Visitor &v) {
        constexpr auto ctx = std::meta::access_context::current();

        // -------- fields --------
        template for (constexpr auto fld :
                      std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
            constexpr auto name = std::define_static_string(std::meta::identifier_of(fld));
            constexpr auto anns = std::define_static_array(std::meta::annotations_of(fld));

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                v.template field<fld, ([:std::meta::constant_of(anns[Is]):])...>(name);
            }(std::make_index_sequence<anns.size()>{});
        }

        // -------- methods (instance + static) --------
        template for (constexpr auto fn :
                      std::define_static_array(std::meta::members_of(^^T, ctx))) {
            if constexpr (is_exportable_member_function(fn)) {
                constexpr auto name = std::define_static_string(std::meta::identifier_of(fn));
                constexpr auto anns = std::define_static_array(std::meta::annotations_of(fn));

                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    if constexpr (std::meta::is_static_member(fn)) {
                        v.template method_static<fn, ([:std::meta::constant_of(anns[Is]):])...>(
                            name);
                    } else {
                        v.template method_instance<fn, ([:std::meta::constant_of(anns[Is]):])...>(
                            name);
                    }
                }(std::make_index_sequence<anns.size()>{});
            }
        }
    }

} // namespace rosetta
