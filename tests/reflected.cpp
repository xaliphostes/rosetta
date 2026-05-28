// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Prototype: Rosetta-style runtime registry, populated automatically
// by a single consteval walk over C++26 reflection metadata.
//
// User code calls `rosetta::register_reflected<T>()` once. No per-field
// or per-method registration. The registry is then queryable at runtime
// by name (string), which is what enables language bindings, GUI editors,
// REST endpoints, etc. on top.

#include <any>
#include <cmath>
#include <experimental/meta>
#include <functional>
#include <print>
#include <span>
#include <string>
#include <string_view>
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
        std::function<Any(void *, std::span<const Any>)> invoke;
    };

    struct ClassMeta {
        std::string                                 name;
        std::unordered_map<std::string, FieldMeta>  fields;
        std::unordered_map<std::string, MethodMeta> methods;
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

        // ---- methods ----
        template for (constexpr auto fn :
                      std::define_static_array(std::meta::members_of(^^T, ctx))) {
            if constexpr (is_exportable_member_function(fn)) {
                constexpr auto arity =
                    std::define_static_array(std::meta::parameters_of(fn)).size();
                std::string mname{std::meta::identifier_of(fn)};
                cls.methods[mname] = MethodMeta{
                    .name = mname,
                    .return_type =
                        std::string(std::meta::display_string_of(std::meta::return_type_of(fn))),
                    .invoke = detail::make_invoker<T, fn>(std::make_index_sequence<arity>{}),
                };
            }
        }
    }

} // namespace rosetta

// =================== demo ===================

struct Vector3 {
    double x, y, z;

    double length() const { return std::sqrt(x * x + y * y + z * z); }
    void   reset() { x = y = z = 0.0; }
    void   scale(double k) {
        x *= k;
        y *= k;
        z *= k;
    }
    double distance_to(double tx, double ty, double tz) const {
        double dx = x - tx, dy = y - ty, dz = z - tz;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

struct Person {
    std::string name;
    int         age;

    std::string greet(const std::string &greeting) const { return greeting + ", " + name + "!"; }
    int         years_until(int target_age) const { return target_age - age; }
    std::string format(const std::string &sep, bool reversed) const {
        return reversed ? std::to_string(age) + sep + name : name + sep + std::to_string(age);
    }
};

template <typename T> void dump(const rosetta::ClassMeta *cls) {
    std::println("=== {} ===", cls->name);
    std::println("fields:");
    for (auto const &[_, f] : cls->fields) {
        std::println("  {} : {}", f.name, f.type_name);
    }
    std::println("methods:");
    for (auto const &[_, m] : cls->methods) {
        std::println("  {}() -> {}", m.name, m.return_type);
    }
}

int main() {
    rosetta::register_reflected<Vector3>();
    rosetta::register_reflected<Person>();

    const auto *vcls = rosetta::Registry::instance().find("Vector3");
    dump<Vector3>(vcls);

    Vector3 v{3.0, 4.0, 0.0};
    std::println("\n-- Vector3 --");
    vcls->fields.at("x").set(&v, rosetta::Any{6.0});
    std::println("after set('x', 6.0): v.x = {}", v.x);

    auto len = vcls->methods.at("length").invoke(&v, {});
    std::println("length() = {}", std::any_cast<double>(len));

    vcls->methods.at("scale").invoke(&v, rosetta::args(0.5));
    std::println("after scale(0.5): v = ({}, {}, {})", v.x, v.y, v.z);

    auto d = vcls->methods.at("distance_to").invoke(&v, rosetta::args(0.0, 0.0, 0.0));
    std::println("distance_to(origin) = {}", std::any_cast<double>(d));

    std::println("\n");
    const auto *pcls = rosetta::Registry::instance().find("Person");
    dump<Person>(pcls);

    Person alice{"Alice", 30};
    std::println("\n-- Person --");
    auto g = pcls->methods.at("greet").invoke(&alice, rosetta::args(std::string("Hello")));
    std::println("{}", std::any_cast<std::string>(g));

    auto y = pcls->methods.at("years_until").invoke(&alice, rosetta::args(40));
    std::println("years_until(40) = {}", std::any_cast<int>(y));

    auto s = pcls->methods.at("format").invoke(&alice, rosetta::args(std::string(" / "), true));
    std::println("format(' / ', true) = {}", std::any_cast<std::string>(s));

    return 0;
}
