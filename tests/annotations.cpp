// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Prototype v4: annotations (P3394) on fields and methods.
//
// On top of v3:
//  - Annotation types live in `rosetta::` (doc, range, readonly).
//  - The walk extracts them via std::meta::annotation_of_type<T>(info), at
//    compile time, and stores the value on FieldMeta / MethodMeta.
//  - The readonly annotation makes the setter throw; the range annotation
//    makes the setter validate before writing.
//  - Requires the additional compile flag: -fannotation-attributes.

#include <algorithm>
#include <any>
#include <cmath>
#include <experimental/meta>
#include <functional>
#include <optional>
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

    // ---- Annotation types (used as [[=rosetta::doc{"..."}]] etc.) ----
    // Each must be a structural type whose value is usable as a template
    // argument. A bare const char* / string_view to a string literal is NOT
    // an NTTP-eligible pointer; we route the literal through
    // std::meta::define_static_string to get a pointer to static storage
    // with linkage.
    struct doc {
        const char *text;
        consteval doc(const char *t) : text(std::define_static_string(t)) {}
        bool operator==(const doc &) const = default;
    };
    struct range {
        double min;
        double max;
        bool operator==(const range &) const = default;
    };
    struct readonly {
        bool operator==(const readonly &) const = default;
    };

    struct FieldMeta {
        std::string                              name;
        std::string                              type_name;
        std::function<Any(void *)>               get;
        std::function<void(void *, const Any &)> set;

        // ---- annotation-derived metadata ----
        std::string                       doc;
        std::optional<std::pair<double, double>> range;  // {min, max}
        bool                              is_readonly = false;
    };

    struct MethodMeta {
        std::string                                      name;
        std::string                                      return_type;
        std::vector<std::type_index>                     param_types;      // for dispatch
        std::vector<std::string>                         param_type_names; // for display
        bool                                             is_static = false;
        std::function<Any(void *, std::span<const Any>)> invoke;

        // ---- annotation-derived metadata ----
        std::string doc;

        std::string signature() const {
            std::string s = (is_static ? "static " : "") + name + "(";
            for (size_t i = 0; i < param_type_names.size(); ++i) {
                if (i) {
                    s += ", ";
                }
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
            if (it == methods.end()) {
                return nullptr;
            }
            for (const auto &m : it->second) {
                if (m.param_types.size() != args.size()) {
                    continue;
                }
                bool ok = true;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (std::type_index(args[i].type()) != m.param_types[i]) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    return &m;
                }
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
                if (i) {
                    msg += ", ";
                }
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

            // ---- fields (with annotations) ----
            template for (constexpr auto m :
                          std::define_static_array(
                              std::meta::nonstatic_data_members_of(^^Self, ctx))) {
                using MemberT = [:std::meta::type_of(m):];
                std::string fname{std::meta::identifier_of(m)};

                // Compile-time annotation extraction.
                constexpr auto doc_opt   = std::meta::annotation_of_type<doc>(m);
                constexpr auto range_opt = std::meta::annotation_of_type<range>(m);
                constexpr bool ro        = std::meta::annotation_of_type<readonly>(m).has_value();

                FieldMeta fm;
                fm.name = fname;
                fm.type_name =
                    std::string(std::meta::display_string_of(std::meta::type_of(m)));
                if constexpr (doc_opt.has_value()) {
                    fm.doc = doc_opt->text;
                }
                if constexpr (range_opt.has_value()) {
                    fm.range = std::make_pair(range_opt->min, range_opt->max);
                }
                fm.is_readonly = ro;

                fm.get = [](void *obj) -> Any {
                    Self *self = static_cast<Self *>(static_cast<Derived *>(obj));
                    return Any{self->[:m:]};
                };

                if constexpr (ro) {
                    fm.set = [fname](void *, const Any &) {
                        throw std::runtime_error("field '" + fname +
                                                 "' is annotated [[=readonly{}]]");
                    };
                } else if constexpr (range_opt.has_value() &&
                                     std::is_arithmetic_v<MemberT>) {
                    constexpr double rmin = range_opt->min;
                    constexpr double rmax = range_opt->max;
                    fm.set = [fname, rmin, rmax](void *obj, const Any &v) {
                        MemberT val = std::any_cast<MemberT>(v);
                        double  d   = static_cast<double>(val);
                        if (d < rmin || d > rmax) {
                            throw std::runtime_error(
                                "field '" + fname + "' value out of [[=range{" +
                                std::to_string(rmin) + ", " +
                                std::to_string(rmax) + "}]] bounds");
                        }
                        Self *self = static_cast<Self *>(static_cast<Derived *>(obj));
                        self->[:m:] = val;
                    };
                } else {
                    fm.set = [](void *obj, const Any &v) {
                        Self *self = static_cast<Self *>(static_cast<Derived *>(obj));
                        self->[:m:] = std::any_cast<MemberT>(v);
                    };
                }

                cls.fields[fname] = std::move(fm);
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

                    constexpr auto m_doc_opt = std::meta::annotation_of_type<doc>(fn);
                    MethodMeta meta{
                        .name = mname,
                        .return_type = std::string(std::meta::display_string_of(
                            std::meta::return_type_of(fn))),
                        .param_types      = std::move(ptypes),
                        .param_type_names = std::move(pnames),
                        .is_static        = std::meta::is_static_member(fn),
                        .invoke           = std::move(invoker),
                    };
                    if constexpr (m_doc_opt.has_value()) {
                        meta.doc = m_doc_opt->text;
                    }

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

// Demo class with annotated fields and methods.
struct Person {
    [[=rosetta::doc{"the person's display name"}]]
    std::string name;

    [[=rosetta::doc{"age in whole years"}, =rosetta::range{0.0, 150.0}]]
    int age = 0;

    [[=rosetta::doc{"server-assigned identifier"}, =rosetta::readonly{}]]
    std::string id;

    Person() = default;
    Person(std::string n, int a, std::string i)
        : name(std::move(n)), age(a), id(std::move(i)) {}

    [[=rosetta::doc{"Returns a greeting prefixed by the given salutation."}]]
    std::string greet(const std::string &salutation) const {
        return salutation + ", " + name + "!";
    }

    void clear() { name.clear(); age = 0; /* id is read-only at the API surface */ }
};

void dump(const rosetta::ClassMeta *cls) {
    std::println("=== {} ===", cls->name);
    if (!cls->bases.empty()) {
        std::print("bases:");
        for (auto const &b : cls->bases) { std::print(" {}", b); }
        std::println("");
    }
    std::println("fields:");
    for (auto const &[_, f] : cls->fields) {
        std::print("  {} : {}", f.name, f.type_name);
        if (f.is_readonly) { std::print(" [readonly]"); }
        if (f.range) {
            std::print(" [range: {}..{}]", f.range->first, f.range->second);
        }
        std::println("");
        if (!f.doc.empty()) { std::println("      doc: {}", f.doc); }
    }
    std::println("methods:");
    for (auto const &[name, overloads] : cls->methods) {
        if (overloads.size() == 1) {
            std::println("  {}", overloads[0].signature());
            if (!overloads[0].doc.empty()) {
                std::println("      doc: {}", overloads[0].doc);
            }
        } else {
            std::println("  {} ({} overloads):", name, overloads.size());
            for (auto const &m : overloads) {
                std::println("    {}", m.signature());
                if (!m.doc.empty()) { std::println("        doc: {}", m.doc); }
            }
        }
    }
}

int main() {
    rosetta::register_reflected<Person>();
    const auto *cls = rosetta::Registry::instance().find("Person");
    dump(cls);

    Person p{"Alice", 30, "p-001"};

    std::println("\n-- read normal field --");
    std::println("name = {}",
                 std::any_cast<std::string>(cls->fields.at("name").get(&p)));

    std::println("\n-- valid range write --");
    cls->fields.at("age").set(&p, rosetta::Any{42});
    std::println("after set('age', 42): age = {}", p.age);

    std::println("\n-- range violation (age = 999) --");
    try {
        cls->fields.at("age").set(&p, rosetta::Any{999});
    } catch (const std::exception &e) {
        std::println("caught: {}", e.what());
    }

    std::println("\n-- readonly violation (id) --");
    try {
        cls->fields.at("id").set(&p, rosetta::Any{std::string("p-999")});
    } catch (const std::exception &e) {
        std::println("caught: {}", e.what());
    }
    std::println("id is unchanged: {}", p.id);

    std::println("\n-- annotated method invocation --");
    auto g = cls->invoke("greet", &p, rosetta::args(std::string("Hello")));
    std::println("greet(\"Hello\") = {}", std::any_cast<std::string>(g));

    return 0;
}
