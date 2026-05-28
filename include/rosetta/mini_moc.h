// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

#pragma once

// =============================================================================
// rosetta/mini_moc.h
//
// A header-only, moc-less reimagining of Qt's signals / slots / properties on
// top of C++26 reflection (P2996) + annotations (P3394).
//
// Requires a toolchain with reflection + annotation-attributes. Tested with the
// Bloomberg clang-p2996 fork.
//
// Notes vs. real Qt:
//  * Token injection (P3294) isn't in clang-p2996 yet, so we cannot auto-emit
//    `name()/setName()` accessors into the class body. Instead, callers reach
//    properties through `get<"name">(obj)` / `set<"name">(obj, v)` — same
//    semantics (equality-gated, fires NOTIFY), just outside the class.
//  * `connect<"sig","slot">(sender, receiver)` is compile-time-checked:
//    wrong name -> static_assert, wrong arg types -> template error.
//
// See tests/moc.cpp
//
// =============================================================================

#include <cstddef>
#include <experimental/meta>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace rosetta::moc {

    // -------------------------------------------------------------------------
    // NTTP-friendly fixed string, used for connect<"sig","slot">/get/set.
    // std::meta::fixed_string doesn't exist in this fork, so we roll our own.
    // -------------------------------------------------------------------------
    template <std::size_t N> struct fixed_string {
        char data[N]{};
        consteval fixed_string(const char (&s)[N]) {
            for (std::size_t i = 0; i < N; ++i) {
                data[i] = s[i];
            }
        }
        constexpr std::string_view view() const {
            return {data, N - 1};
        }
        bool operator==(const fixed_string &) const = default;
    };
    template <std::size_t N> fixed_string(const char (&)[N]) -> fixed_string<N>;

    // -------------------------------------------------------------------------
    // Annotation tag types.
    // -------------------------------------------------------------------------
    struct signal_tag {
        bool operator==(const signal_tag &) const = default;
    };

    struct slot_tag {
        bool operator==(const slot_tag &) const = default;
    };

    inline constexpr signal_tag signal{};
    inline constexpr slot_tag   slot{};

    // -------------------------------------------------------------------------
    // Property annotation. `name` is the handle used by get<>/set<>;
    // `notify` is an optional sibling Signal member fired after a write that
    // actually changed the value.
    //
    // The string literals are routed through define_static_string so the
    // resulting const char* is NTTP-eligible (pointer to static storage with
    // linkage), which the annotation machinery requires.
    // -------------------------------------------------------------------------
    struct property {
        const char *name;
        const char *notify;
        consteval property(const char *n, const char *nf = ""): 
            name(std::define_static_string(n)),
            notify(std::define_static_string(nf)) {}
        bool operator==(const property &) const = default;
    };

    // -------------------------------------------------------------------------
    // Signal<Args...>: minimal type-safe multicast.
    // -------------------------------------------------------------------------
    template <class... Args> class Signal {
        std::vector<std::function<void(Args...)>> subs_;

    public:
        void connect(std::function<void(Args...)> f) {
            subs_.push_back(std::move(f));
        }
        void operator()(Args... a) const {
            for (auto const &f : subs_) {
                f(a...);
            }
        }
    };

    // -------------------------------------------------------------------------
    // Reflection helpers.
    // -------------------------------------------------------------------------
    namespace detail {

        template <class Tag, class T>
        consteval std::meta::info find_tagged(std::string_view name) {
            constexpr auto ctx = std::meta::access_context::unchecked();
            for (auto m : std::meta::members_of(^^T, ctx)) {
                if (std::meta::annotation_of_type<Tag>(m).has_value() && std::meta::identifier_of(m) == name) {
                    return m;
                }
            }
            return {};
        }

        template <class T>
        consteval std::meta::info find_property_field(std::string_view name) {
            constexpr auto ctx = std::meta::access_context::unchecked();
            for (auto m : std::meta::nonstatic_data_members_of(^^T, ctx)) {
                auto pa = std::meta::annotation_of_type<property>(m);
                if (pa.has_value() && std::string_view{pa->name} == name) {
                    return m;
                }
            }
            return {};
        }

    } // namespace detail

    // -------------------------------------------------------------------------
    // connect<"sig","slot">(sender, receiver)
    //
    // Compile-time-checked: missing names static_assert, mismatched argument
    // types fail at the lambda body.
    // -------------------------------------------------------------------------
    template <fixed_string Sig, fixed_string Slot, class S, class R>
    void connect(S &sender, R &receiver) {
        constexpr auto sig_r  = detail::find_tagged<signal_tag, S>(Sig.view());
        constexpr auto slot_r = detail::find_tagged<slot_tag, R>(Slot.view());
        static_assert(sig_r != std::meta::info{}, "no [[=signal]]-tagged member with that name on sender");
        static_assert(slot_r != std::meta::info{}, "no [[=slot]]-tagged member with that name on receiver");

        (sender.[:sig_r:]).connect([&receiver](auto &&...args) {
            (receiver.[:slot_r:])(std::forward<decltype(args)>(args)...);
        });
    }

    // -------------------------------------------------------------------------
    // get<"prop">(obj): read access for a [[=property]]-annotated field.
    // -------------------------------------------------------------------------
    template <fixed_string Name, class T> auto const &get(T const &obj) {
        constexpr auto field_r = detail::find_property_field<T>(Name.view());
        static_assert(field_r != std::meta::info{}, "no [[=property]] with that handle on T");
        return obj.[:field_r:];
    }

    // -------------------------------------------------------------------------
    // set<"prop">(obj, value): write access. Equality-gated; if the property
    // declares a NOTIFY signal, fires it after a successful change.
    // -------------------------------------------------------------------------
    template <fixed_string Name, class T, class V>
    void set(T &obj, V &&value) {
        constexpr auto field_r = detail::find_property_field<T>(Name.view());
        static_assert(field_r != std::meta::info{}, "no [[=property]] with that handle on T");

        using F = [:std::meta::type_of(field_r):];
        F v(std::forward<V>(value));
        if (obj.[:field_r:] == v) {
            return;
        }
        obj.[:field_r:] = std::move(v);

        constexpr auto pa = std::meta::annotation_of_type<property>(field_r).value();
        constexpr auto notify_name = std::string_view{pa.notify};
        if constexpr (!notify_name.empty()) {
            constexpr auto sig_r = detail::find_tagged<signal_tag, T>(notify_name);
            static_assert(sig_r != std::meta::info{}, "NOTIFY signal named by property is not declared");
            (obj.[:sig_r:])(obj.[:field_r:]);
        }
    }

} // namespace rosetta::moc
