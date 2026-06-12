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
#include <list>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace rosetta::moc {

    /**
     * fixed_string: a simple wrapper to allow string literals as NTTPs for annotation parameters.
     * The annotation machinery requires that the parameter be a pointer to static storage with
     * linkage, so we route the string literals through define_static_string to ensure they meet
     * that requirement. The view() method provides a convenient way to get a std::string_view of
     * the fixed_string, which is what the rest of the code uses for comparisons and lookups. We
     * also define an equality operator for fixed_string to allow for easy comparison in
     * static_asserts and other compile-time checks.
     * Note that fixed_string is a simple wrapper around a char array, and it doesn't manage memory
     * or provide any dynamic features. It's purely a compile-time utility for working with string
     * literals in the context of annotations and NTTPs.
     */
    template <std::size_t N> struct fixed_string {
        char data[N]{};
        consteval fixed_string(const char (&s)[N]) {
            for (std::size_t i = 0; i < N; ++i) {
                data[i] = s[i];
            }
        }
        constexpr std::string_view view() const { return {data, N - 1}; }
        bool                       operator==(const fixed_string &) const = default;
    };
    template <std::size_t N> fixed_string(const char (&)[N]) -> fixed_string<N>;

    /**
     * signal_tag and slot_tag: empty structs used as annotation tags to mark members as signals or
     * slots. The operator== is defaulted to allow for easy comparison in static_asserts and other
     * compile-time checks. These tags are used in the find_tagged function to identify which
     * members of a class are annotated as signals or slots. By defining these tags as empty
     * structs, we can use them purely as markers without any associated data, and the defaulted
     * equality operator allows us to compare them easily when searching for annotated members.
     */
    struct signal_tag {
        bool operator==(const signal_tag &) const = default;
    };

    /**
     * slot_tag is an empty struct used as an annotation tag to mark members as slots. The
     * operator== is defaulted to allow for easy comparison in static_asserts and other compile-time
     * checks. This tag is used in the find_tagged function to identify which members of a class are
     * annotated as slots. By defining this tag as an empty struct, we can use it purely as a marker
     * without any associated data, and the defaulted equality operator allows us to compare it
     * easily when searching for annotated members.
     */
    struct slot_tag {
        bool operator==(const slot_tag &) const = default;
    };

    inline constexpr signal_tag signal{};
    inline constexpr slot_tag   slot{};

    /**
     * property: an annotation struct used to mark fields as properties. It contains the name of the
     * property and an optional notify signal name. The operator== is defaulted to allow for easy
     * comparison in static_asserts and other compile-time checks. This annotation is used in the
     * find_property_field function to identify which fields of a class are annotated as properties.
     * By defining this struct with a constructor that takes the property name and an optional
     * notify signal name, we can easily annotate fields with the necessary metadata for property
     * access and change notification. The defaulted equality operator allows us to compare property
     * annotations easily when searching for annotated fields. Note that the property annotation is
     * designed to be used on fields, and the find_property_field function assumes that the
     * annotation is on the field itself, not on the getter/setter. This design choice simplifies
     * the implementation and is consistent with the idea of a "property" as a field with some extra
     * metadata. If we wanted to support annotations on getters/setters, we would need to look at
     * member functions as well and figure out which one is the "main" one for the property, which
     * would add complexity to the implementation.
     */
    struct property {
        const char *name;
        const char *notify;
        consteval property(const char *n, const char *nf = "")
            : name(std::define_static_string(n)), notify(std::define_static_string(nf)) {}
        bool operator==(const property &) const = default;
    };

    /**
     * ScopedConnection: an RAII handle that ties a single signal/slot connection to a scope (or to
     * an owning object). When the ScopedConnection is destroyed it disconnects the connection it
     * owns, so callers no longer have to remember to call Signal::disconnect(id) by hand. This is the
     * standard fix for the "dangling slot" hazard: a slot that captures a receiver outlives that
     * receiver and the next emission calls into freed memory. Storing a ScopedConnection as a member
     * of the receiver makes the connection die with the receiver automatically.
     *
     * The disconnect action is type-erased into a std::function<void()>, so one (non-templated)
     * ScopedConnection type can own a connection to any Signal<Args...>. It is move-only: a
     * connection has exactly one owner, and copying would let two objects both try to disconnect the
     * same Id. reset() disconnects early; release() detaches ownership without disconnecting (the
     * connection then lives on, like std::unique_ptr::release).
     *
     * Lifetime caveat: a ScopedConnection captures a back-reference to its Signal, so the Signal must
     * outlive any ScopedConnection that targets it; otherwise ~ScopedConnection would disconnect on a
     * dead object. For this prototype that ordering is documented rather than enforced.
     */
    class ScopedConnection {
        std::function<void()> disconnect_;

    public:
        ScopedConnection() = default;
        explicit ScopedConnection(std::function<void()> d);

        ScopedConnection(ScopedConnection &&o) noexcept;
        ScopedConnection &operator=(ScopedConnection &&o) noexcept;
        ScopedConnection(const ScopedConnection &)            = delete;
        ScopedConnection &operator=(const ScopedConnection &) = delete;

        ~ScopedConnection();

        /// Disconnect now, if still owning a live connection.
        void reset();

        /// Detach ownership without disconnecting; the connection survives.
        void release();

        /// True while this handle still owns a connection.
        bool connected() const;
    };

    /**
     * Signal: a simple implementation of a signal class that can connect to multiple subscribers.
     * It uses a vector of std::function to store the connected slots, and the connect method allows
     * adding new subscribers. The operator() is defined to call all connected slots with the
     * provided arguments. This class serves as a basic building block for the signal-slot
     * mechanism, allowing us to define signals that can be emitted and have multiple slots respond
     * to them. The template parameters allow us to define signals with any number of arguments, and
     * the use of std::function provides flexibility in the types of callables that can be connected
     * as slots.
     *
     * Disconnection: connect() returns a stable Id (a monotonically increasing handle) that
     * identifies the connection independently of its position in the underlying vector. disconnect()
     * removes the connection with that Id, and disconnect_all() removes every connection. Stable Ids
     * are necessary because vector indices shift whenever an earlier connection is erased, and
     * std::function itself is not equality-comparable, so there would otherwise be nothing to
     * disconnect by. Emission is re-entrancy-safe: a slot may disconnect itself (or another slot)
     * while the signal is firing without invalidating the in-progress iteration.
     *
     * Note that this implementation still does not handle return values from slots or manage object
     * lifetimes, but it serves the purpose of demonstrating the core concept of signals and slots in
     * the context of this mini_moc implementation.
     */
    template <class... Args> class Signal {
        struct Conn {
            std::size_t                  id;
            std::function<void(Args...)> fn; // null == tombstoned, pending removal
        };
        // Node-based storage: a slot may connect or disconnect during emission, and
        // std::list keeps every other node's iterator valid across both, so we never
        // move or destroy the std::function we are currently calling.
        mutable std::list<Conn> subs_;
        mutable std::size_t     emitting_{0};
        std::size_t             next_{1};

        // Physically drop tombstoned connections. Only safe when not iterating.
        void compact() const;

    public:
        /// Opaque handle identifying a single connection. 0 is never a valid Id.
        using Id = std::size_t;

        /// Connect a slot; returns a stable Id that can be passed to disconnect().
        Id connect(std::function<void(Args...)> f);

        /// Remove the connection with the given Id. Returns true if one was removed.
        bool disconnect(Id id);

        /// Remove every connection.
        void disconnect_all();

        /// Connect a slot and hand back an RAII handle that disconnects it on destruction.
        ScopedConnection scoped_connect(std::function<void(Args...)> f);

        void operator()(Args... a) const;
    };

    /**
     * connect<"sig","slot">(sender, receiver): a function template that connects a signal from the
     * sender to a slot on the receiver. It uses compile-time reflection to find the members of the
     * sender and receiver that are annotated with the specified signal and slot tags. If the
     * specified signal or slot is not found, it triggers a static_assert with an appropriate error
     * message. If both are found, it connects the signal to the slot by creating a lambda that
     * captures the receiver and calls the slot with the arguments from the signal. This function
     * provides a compile-time-checked way to connect signals and slots, ensuring that the specified
     * signal and slot exist and have compatible argument types. The use of fixed_string for the
     * signal and slot names allows us to use string literals as template parameters, which are then
     * used in the reflection queries to find the corresponding members.
     * Note that the actual connection is done by calling the connect method on the signal member of
     * the sender, passing a lambda that captures the receiver and calls the slot member of the
     * receiver. This means that the signal and slot members must be callable (e.g., the signal
     * member should be of type Signal<Args...> and the slot member should be a callable that can
     * accept the same arguments).
     *
     * Returns the Signal::Id of the freshly created connection, so the caller can later tear it down
     * with `(sender.theSignal).disconnect(id)`.
     */
    template <fixed_string Sig, fixed_string Slot, class S, class R>
    auto connect(S &sender, R &receiver);

    /**
     * get<"prop">(obj): read access to a property. It uses compile-time reflection to find the
     * field of the object that is annotated with the specified property name. If the property is
     * not found or if the annotation is not present, it triggers a static_assert with an
     * appropriate error message. If the property is found, it returns a const reference to the
     * field. This function provides a way to access properties that are annotated with the property
     * annotation, allowing us to read their values using a simple get function. The use of
     * fixed_string for the property name allows us to use string literals as template parameters,
     * which are then used in the reflection queries to find the corresponding field.
     * Note that this function assumes that the property annotation is on the field itself, and it
     * does not support properties that are defined through getters/setters. This design choice
     * simplifies the implementation and is consistent with the idea of a "property" as a field with
     * some extra metadata. If we wanted to support properties defined through getters/setters, we
     * would need to look at member functions as well and figure out which one is the "main" one for
     * the property, which would add complexity to the implementation.
     */
    template <fixed_string Name, class T> auto const &get(T const &obj);

    /**
     * set<"prop">(obj, value): write access to a property. It uses compile-time reflection to find
     * the field of the object that is annotated with the specified property name. If the property
     * is not found or if the annotation is not present, it triggers a static_assert with an
     * appropriate error message. If the property is found, it sets the field to the provided value.
     * This function provides a way to modify properties that are annotated with the property
     * annotation, allowing us to write their values using a simple set function. The use of
     * fixed_string for the property name allows us to use string literals as template parameters,
     * which are then used in the reflection queries to find the corresponding field.
     */
    template <fixed_string Name, class T, class V> void set(T &obj, V &&value);

} // namespace rosetta::moc

#include "inline/mini_moc.hxx"
