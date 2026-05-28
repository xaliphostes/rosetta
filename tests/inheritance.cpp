// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Prototype v3: overload resolution + inheritance walk + static methods.
//
// On top of v2:
//  - ClassMeta::bases records the chain of registered base classes.
//  - Members of base classes are flattened into the derived ClassMeta, with
//    field/method invokers that perform the implicit derived-to-base cast.
//  - is_static_member(fn) routes to a separate invoker that calls the
//    function without an object pointer.

#include <algorithm>
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
        bool                                             is_static = false;
        std::function<Any(void *, std::span<const Any>)> invoke;

        std::string signature() const {
            std::string s = (is_static ? "static " : "") + name + "(";
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
        std::vector<std::string>                                 bases;  // names of registered bases
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

    // Filter out non-exportable member functions: keep regular methods and
    // static methods, drop constructors/destructors/compiler-generated specials.
    consteval bool is_exportable_member_function(std::meta::info fn) {
        return std::meta::is_function(fn) && !std::meta::is_constructor(fn) &&
               !std::meta::is_destructor(fn) && !std::meta::is_special_member_function(fn);
    }

    namespace detail {

        // Instance invoker. Derived is the type the registry was opened for
        // (what void* actually points to); Owner is the class that declares
        // the member function (== Derived for the class's own methods, or an
        // ancestor when this method was inherited).
        template <typename Derived, typename Owner, std::meta::info Fn, std::size_t... Is>
        auto make_instance_invoker(std::index_sequence<Is...>) {
            return [](void *obj, std::span<const Any> args) -> Any {
                using R               = [:std::meta::return_type_of(Fn):];
                constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));

                // implicit Derived* -> Owner* conversion through static_cast
                Owner *self = static_cast<Owner *>(static_cast<Derived *>(obj));

                if constexpr (std::is_void_v<R>) {
                    (self->[:Fn:])(
                        std::any_cast<
                            std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                            args[Is])...);
                    return Any{};
                } else {
                    return Any{(self->[:Fn:])(
                        std::any_cast<
                            std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                            args[Is])...)};
                }
            };
        }

        // Static invoker. obj is ignored.
        template <std::meta::info Fn, std::size_t... Is>
        auto make_static_invoker(std::index_sequence<Is...>) {
            return [](void * /*obj*/, std::span<const Any> args) -> Any {
                using R               = [:std::meta::return_type_of(Fn):];
                constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));

                if constexpr (std::is_void_v<R>) {
                    [:Fn:](
                        std::any_cast<
                            std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                            args[Is])...);
                    return Any{};
                } else {
                    return Any{[:Fn:](
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

    namespace detail {

        // Adds Self's own members into cls, with void* cast as Derived* then
        // implicit conversion to Self* (for base members reached via Derived).
        template <typename Derived, typename Self> void register_self_only(ClassMeta &cls) {
            constexpr auto ctx = std::meta::access_context::current();

            // ---- fields ----
            template for (constexpr auto m :
                          std::define_static_array(
                              std::meta::nonstatic_data_members_of(^^Self, ctx))) {
                using MemberT = [:std::meta::type_of(m):];
                std::string fname{std::meta::identifier_of(m)};
                cls.fields[fname] = FieldMeta{
                    .name      = fname,
                    .type_name = std::string(std::meta::display_string_of(std::meta::type_of(m))),
                    .get =
                        [](void *obj) -> Any {
                            Self *self = static_cast<Self *>(static_cast<Derived *>(obj));
                            return Any{self->[:m:]};
                        },
                    .set =
                        [](void *obj, const Any &v) {
                            Self *self = static_cast<Self *>(static_cast<Derived *>(obj));
                            self->[:m:] = std::any_cast<MemberT>(v);
                        },
                };
            }

            // ---- methods (instance and static) ----
            template for (constexpr auto fn :
                          std::define_static_array(std::meta::members_of(^^Self, ctx))) {
                if constexpr (is_exportable_member_function(fn)) {
                    constexpr auto arity =
                        std::define_static_array(std::meta::parameters_of(fn)).size();
                    std::string mname{std::meta::identifier_of(fn)};
                    auto [ptypes, pnames] =
                        build_param_info<fn>(std::make_index_sequence<arity>{});

                    auto invoker = [&]() {
                        if constexpr (std::meta::is_static_member(fn)) {
                            return make_static_invoker<fn>(std::make_index_sequence<arity>{});
                        } else {
                            return make_instance_invoker<Derived, Self, fn>(
                                std::make_index_sequence<arity>{});
                        }
                    }();

                    MethodMeta meta{
                        .name = mname,
                        .return_type = std::string(std::meta::display_string_of(
                            std::meta::return_type_of(fn))),
                        .param_types      = std::move(ptypes),
                        .param_type_names = std::move(pnames),
                        .is_static        = std::meta::is_static_member(fn),
                        .invoke           = std::move(invoker),
                    };

                    // If an overload with the same signature already exists
                    // (from a base walk), the derived registration replaces it.
                    auto &bucket = cls.methods[mname];
                    auto  same   = std::find_if(bucket.begin(), bucket.end(),
                                                [&](const MethodMeta &m) {
                                                  return m.param_types == meta.param_types;
                                              });
                    if (same != bucket.end()) {
                        *same = std::move(meta);
                    } else {
                        bucket.push_back(std::move(meta));
                    }
                }
            }
        }

        // Recursive depth-first walk: bases first, then Self's own members.
        template <typename Derived, typename Self> void register_with_bases(ClassMeta &cls) {
            constexpr auto ctx = std::meta::access_context::current();
            template for (constexpr auto b :
                          std::define_static_array(std::meta::bases_of(^^Self, ctx))) {
                using BaseT = [:std::meta::type_of(b):];
                cls.bases.push_back(
                    std::string(std::meta::identifier_of(std::meta::type_of(b))));
                register_with_bases<Derived, BaseT>(cls);
            }
            register_self_only<Derived, Self>(cls);
        }

    } // namespace detail

    template <typename T> void register_reflected() {
        constexpr std::string_view type_name = std::meta::identifier_of(^^T);
        ClassMeta &cls = Registry::instance().get_or_create(type_name);
        cls.name       = std::string(type_name);
        detail::register_with_bases<T, T>(cls);
    }

} // namespace rosetta

// =================== demo ===================

// Hierarchy: Shape (with a static factory id), then Circle and Square as derived.
struct Shape {
    std::string name;

    Shape() = default;
    explicit Shape(std::string n) : name(std::move(n)) {}

    std::string    describe() const { return "I am a " + name; }
    virtual double area() const { return 0.0; }

    // Static method — invoked without an instance
    static int next_id() { return ++id_counter_; }

private:
    static inline int id_counter_ = 0;
};

struct Circle : Shape {
    double radius = 0.0;

    Circle() = default;
    Circle(std::string n, double r) : Shape(std::move(n)), radius(r) {}

    double area() const override { return 3.14159265 * radius * radius; }

    // Static method on the derived class
    static Circle unit() { return Circle{"unit-circle", 1.0}; }
};

struct Square : Shape {
    double side = 0.0;

    Square() = default;
    Square(std::string n, double s) : Shape(std::move(n)), side(s) {}

    double area() const override { return side * side; }
};

void dump(const rosetta::ClassMeta *cls) {
    std::println("=== {} ===", cls->name);
    if (!cls->bases.empty()) {
        std::print("bases:");
        for (auto const &b : cls->bases) std::print(" {}", b);
        std::println("");
    }
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
    rosetta::register_reflected<Shape>();
    rosetta::register_reflected<Circle>();
    rosetta::register_reflected<Square>();

    auto &reg = rosetta::Registry::instance();
    dump(reg.find("Shape"));
    std::println("");
    dump(reg.find("Circle"));
    std::println("");
    dump(reg.find("Square"));

    std::println("\n-- inherited field access --");
    Circle c{"my-circle", 2.0};
    const auto *ccls = reg.find("Circle");
    ccls->fields.at("name").set(&c, rosetta::Any{std::string("renamed-circle")});
    std::println("Shape::name (via Circle) = {}", c.name);
    std::println("Circle::radius           = {}",
                 std::any_cast<double>(ccls->fields.at("radius").get(&c)));

    std::println("\n-- inherited method + virtual override --");
    auto d = ccls->invoke("describe", &c, {});
    std::println("c.describe()             = {}", std::any_cast<std::string>(d));
    auto a = ccls->invoke("area", &c, {});
    std::println("c.area()                 = {}", std::any_cast<double>(a));

    std::println("\n-- static method (no instance) --");
    auto id1 = ccls->invoke("next_id", nullptr, {});
    auto id2 = ccls->invoke("next_id", nullptr, {});
    std::println("Shape::next_id() (twice) = {}, {}",
                 std::any_cast<int>(id1), std::any_cast<int>(id2));

    auto unit = ccls->invoke("unit", nullptr, {});
    Circle u   = std::any_cast<Circle>(unit);
    std::println("Circle::unit()           = ({}, r={})", u.name, u.radius);

    std::println("\n-- sibling class --");
    Square s{"sq", 3.0};
    const auto *scls = reg.find("Square");
    std::println("Square::area()           = {}",
                 std::any_cast<double>(scls->invoke("area", &s, {})));

    return 0;
}
