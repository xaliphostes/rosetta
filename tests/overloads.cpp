// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Prototype v2: same as register_reflected.cpp, plus overload resolution.
//
// Each MethodMeta now carries its parameter types as std::type_index so
// the runtime can dispatch by-signature. ClassMeta::methods becomes
// unordered_map<name, vector<MethodMeta>> — one bucket per overload set.

#include <any>
#include <cmath>
#include <experimental/meta>
#include <functional>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace rosetta {

    using Any = std::any;

    struct FieldMeta {
        std::string                              name;
        std::string                              type_name;
        std::function<Any(void *)>               get;
        std::function<void(void *, const Any &)> set;
    };

    struct MethodMeta {
        std::string                                      name;
        std::string                                      return_type;
        std::vector<std::type_index>                     param_types;      // for dispatch
        std::vector<std::string>                         param_type_names; // for display
        std::function<Any(void *, std::span<const Any>)> invoke;

        std::string signature() const {
            std::string s = name + "(";
            for (size_t i = 0; i < param_type_names.size(); ++i) {
                if (i)
                    s += ", ";
                s += param_type_names[i];
            }
            s += ") -> " + return_type;
            return s;
        }
    };

    struct ClassMeta {
        std::string                                              name;
        std::unordered_map<std::string, FieldMeta>               fields;
        std::unordered_map<std::string, std::vector<MethodMeta>> methods;

        // Resolve overload by matching arg type_indexes against each candidate.
        const MethodMeta *resolve(std::string_view mname, std::span<const Any> args) const {
            auto it = methods.find(std::string(mname));
            if (it == methods.end())
                return nullptr;
            for (const auto &m : it->second) {
                if (m.param_types.size() != args.size())
                    continue;
                bool ok = true;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (std::type_index(args[i].type()) != m.param_types[i]) {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                    return &m;
            }
            return nullptr;
        }

        Any invoke(std::string_view mname, void *obj, std::span<const Any> args) const {
            if (const auto *m = resolve(mname, args)) {
                return m->invoke(obj, args);
            }
            std::string msg = "no overload of '";
            msg += mname;
            msg += "' matches the given argument types: (";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i)
                    msg += ", ";
                msg += args[i].type().name();
            }
            msg += ")";
            throw std::runtime_error(msg);
        }
    };

    class Registry {
    public:
        static Registry &instance() {
            static Registry r;
            return r;
        }
        // Get existing or create new ClassMeta for a given type name
        ClassMeta       &get_or_create(std::string_view n) { return classes_[std::string(n)]; }
        const ClassMeta *find(std::string_view n) const {
            auto it = classes_.find(std::string(n));
            return it == classes_.end() ? nullptr : &it->second;
        }

    private:
        std::unordered_map<std::string, ClassMeta> classes_;
    };

    // Filter out non-exportable member functions: we only want regular methods,
    // not constructors, destructors, operators, static members, or compiler-generated
    // special member functions (copy/move ctors and assignment).
    consteval bool is_exportable_member_function(std::meta::info fn) {
        return std::meta::is_function(fn) && !std::meta::is_constructor(fn) &&
               !std::meta::is_destructor(fn) && !std::meta::is_special_member_function(fn) &&
               !std::meta::is_static_member(fn);
    }

    namespace detail {

        // Stateless invoker: splices the function and each parameter's type,
        // unpacks the Any span by index, calls the method, wraps the result.
        template <typename T, std::meta::info Fn, std::size_t... Is>
        auto make_invoker(std::index_sequence<Is...>) {
            return [](void *obj, std::span<const Any> args) -> Any {
                using R               = [:std::meta::return_type_of(Fn):];
                constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));

                // Return void
                if constexpr (std::is_void_v<R>) {
                    (static_cast<T *>(obj)->[:Fn:])(
                        std::any_cast<
                            std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                            args[Is])...);
                    return Any{};
                }
                // Return something
                else {
                    return Any{(static_cast<T *>(obj)->[:Fn:])(
                        std::any_cast<
                            std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                            args[Is])...)};
                }
            };
        }

        // consteval helper so the info value (a consteval-only type) never
        // escapes into a runtime expression context.
        template <std::meta::info Fn, std::size_t I>
        consteval std::string_view param_type_display_name() {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            return std::meta::display_string_of(std::meta::type_of(params[I]));
        }

        // Build the runtime param_types and param_type_names for one overload.
        template <std::meta::info Fn, std::size_t... Is>
        std::pair<std::vector<std::type_index>, std::vector<std::string>>
        build_param_info(std::index_sequence<Is...>) {
            std::vector<std::type_index> types = {std::type_index(
                typeid(std::remove_cvref_t<typename[:std::meta::type_of(
                    std::define_static_array(std::meta::parameters_of(Fn))[Is]):]>))...};
            std::vector<std::string> names = {
                std::string(param_type_display_name<Fn, Is>())...};
            return {std::move(types), std::move(names)};
        }

    } // namespace detail

    // Pack helpers for callers: rosetta::args(1, "hi", true) -> std::array<Any, 3>
    template <typename... Args> auto args(Args &&...a) {
        return std::array<Any, sizeof...(Args)>{Any{std::forward<Args>(a)}...};
    }

    template <typename T> void register_reflected() {
        constexpr auto             ctx       = std::meta::access_context::current();
        constexpr std::string_view type_name = std::meta::identifier_of(^^T);

        ClassMeta &cls = Registry::instance().get_or_create(type_name);
        cls.name       = std::string(type_name);

        // ---- fields ----
        template for (constexpr auto m :
                      std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
            using MemberT = [:std::meta::type_of(m):];
            std::string fname{std::meta::identifier_of(m)};
            cls.fields[fname] = FieldMeta{
                .name      = fname,
                .type_name = std::string(std::meta::display_string_of(std::meta::type_of(m))),
                .get       = [](void *obj) -> Any { return Any{static_cast<T *>(obj)->[:m:]}; },
                .set =
                    [](void *obj, const Any &v) {
                        static_cast<T *>(obj)->[:m:] = std::any_cast<MemberT>(v);
                    },
            };
        }

        // ---- methods (push into overload bucket per name) ----
        template for (constexpr auto fn :
                      std::define_static_array(std::meta::members_of(^^T, ctx))) {
            if constexpr (is_exportable_member_function(fn)) {
                constexpr auto arity =
                    std::define_static_array(std::meta::parameters_of(fn)).size();
                std::string mname{std::meta::identifier_of(fn)};
                auto [ptypes, pnames] =
                    detail::build_param_info<fn>(std::make_index_sequence<arity>{});
                cls.methods[mname].push_back(MethodMeta{
                    .name = mname,
                    .return_type =
                        std::string(std::meta::display_string_of(std::meta::return_type_of(fn))),
                    .param_types      = std::move(ptypes),
                    .param_type_names = std::move(pnames),
                    .invoke = detail::make_invoker<T, fn>(std::make_index_sequence<arity>{}),
                });
            }
        }
    }

} // namespace rosetta

// =================== demo ===================

struct Calc {
    // Overloaded add — same name, three signatures
    int         add(int a, int b) const { return a + b; }
    double      add(double a, double b) const { return a + b; }
    std::string add(const std::string &a, const std::string &b) const { return a + b; }

    // Overloaded negate — same name, two signatures, different arity from add
    int    negate(int x) const { return -x; }
    double negate(double x) const { return -x; }

    // Non-overloaded — should still work
    void clear() { /* no-op */ }
};

void dump(const rosetta::ClassMeta *cls) {
    std::println("=== {} ===", cls->name);
    std::println("fields:");
    for (auto const &[_, f] : cls->fields) {
        std::println("  {} : {}", f.name, f.type_name);
    }
    std::println("methods:");
    for (auto const &[name, overloads] : cls->methods) {
        if (overloads.size() == 1) {
            std::println("  {}", overloads[0].signature());
        } else {
            std::println("  {} ({} overloads):", name, overloads.size());
            for (auto const &m : overloads) {
                std::println("    {}", m.signature());
            }
        }
    }
}

int main() {
    rosetta::register_reflected<Calc>();
    const auto *cls = rosetta::Registry::instance().find("Calc");
    dump(cls);

    Calc c;
    std::println("\n-- dispatch by overload --");

    auto sum_i = cls->invoke("add", &c, rosetta::args(1, 2));
    std::println("add(1, 2)                 -> {} (int)", std::any_cast<int>(sum_i));

    auto sum_d = cls->invoke("add", &c, rosetta::args(1.5, 2.5));
    std::println("add(1.5, 2.5)             -> {} (double)", std::any_cast<double>(sum_d));

    auto sum_s = cls->invoke("add", &c, rosetta::args(std::string("foo"), std::string("bar")));
    std::println("add(\"foo\", \"bar\")         -> {} (string)", std::any_cast<std::string>(sum_s));

    auto neg_i = cls->invoke("negate", &c, rosetta::args(42));
    std::println("negate(42)                -> {} (int)", std::any_cast<int>(neg_i));

    auto neg_d = cls->invoke("negate", &c, rosetta::args(2.5));
    std::println("negate(2.5)               -> {} (double)", std::any_cast<double>(neg_d));

    cls->invoke("clear", &c, {});
    std::println("clear()                   -> (void)");

    std::println("\n-- bad dispatch --");
    try {
        cls->invoke("add", &c, rosetta::args(true, false));
    } catch (const std::exception &e) {
        std::println("caught: {}", e.what());
    }

    return 0;
}
