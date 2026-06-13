// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// One consteval walk over T's reflected members. Each field/method is
// passed to the visitor together with its *full annotation pack* as
// non-type template parameters; the visitor decides which annotations
// it cares about — there is no per-shape entry-point in the walker.
//
// Visitor concept — `consteval`-template members:
//
//   v.template field          <Fld,  Anns...>(name);
//   v.template method_instance<Fn,   Anns...>(name);
//   v.template method_static  <Fn,   Anns...>(name);
//   v.template constructor    <Ctor, Anns...>();      // optional
//
// `constructor` is optional: the walk only calls it when the visitor
// defines it (detected with `requires`), so backends that don't model
// construction need not implement it. `Ctor` is the `std::meta::info`
// of a public, non-copy/move constructor.
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
//
// This header declares the public surface; the definitions live in
// inline/walk.hxx (included at the bottom).

#pragma once

#include <experimental/meta>
#include <rosetta/annotate.h>
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
        template <typename A> consteval bool has(auto... anns);

        /**
         * @brief Get the first annotation of type A in the pack, or return a fallback value if absent.
         */
        template <typename A> consteval A get_or(A fallback, auto... anns);

    } // namespace ann

    // Keep regular and static methods; drop constructors/destructors and
    // the compiler-generated copy/move specials.
    consteval bool is_exportable_member_function(std::meta::info fn);

    // Expose user-callable constructors: default and parameterized ones, but
    // not copy/move (those are value-passing, not distinct entry points) nor
    // constructor templates (their parameters aren't a fixed pack to splice)
    // nor deleted ones.
    consteval bool is_exportable_constructor(std::meta::info fn);

    /**
     * @brief Walk over reflected members of T and pass them to the visitor.
     */
    template <typename T, typename Visitor> void walk(Visitor &v);

} // namespace rosetta

#include "inline/walk.hxx"
