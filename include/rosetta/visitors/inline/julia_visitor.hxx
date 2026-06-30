namespace rosetta {

    // jlcxx-specific helpers live in their own namespace so the type-trait
    // and predicate names don't collide with the equivalents in other
    // visitors (e.g. node_visitor's rosetta::is_std_vector) when more than
    // one binding kit is pulled into the same translation unit.
    namespace julia_detail {

        // ---- Type classification ----

        template <typename T> struct is_std_vector : std::false_type {};
        template <typename U, typename A>
        struct is_std_vector<std::vector<U, A>> : std::true_type {};

        template <typename T> struct is_std_function : std::false_type {};
        template <typename R, typename... A>
        struct is_std_function<std::function<R(A...)>> : std::true_type {};

        // Can a value of T cross the jlcxx boundary? Scalars, strings,
        // vectors of supported types, enums (as bits types), and registered
        // user class types are convertible; std::function and other unhandled
        // types are not. Mirrors node_visitor's napi_supported.
        template <typename T> consteval bool julia_supported() {
            using U = std::remove_cvref_t<T>;
            if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, bool> ||
                          std::is_arithmetic_v<U>) {
                return true;
            } else if constexpr (is_std_function<U>::value) {
                return false;
            } else if constexpr (is_std_vector<U>::value) {
                // Needs <jlcxx/stl.hpp>, which doesn't yet build against the
                // fork's experimental libc++ — skip vectors for now. See the
                // note in julia_visitor.h.
                return false;
            } else if constexpr (std::is_enum_v<U>) {
                return true; // bits type
            } else if constexpr (std::is_class_v<U>) {
                return true; // user type → registered jlcxx type
            } else {
                return false;
            }
        }

        // A method is bindable only if its return type and every parameter
        // type can cross the boundary; otherwise the walk skips it.
        template <std::meta::info Fn> consteval bool method_supported() {
            using Ret = [:std::meta::return_type_of(Fn):];
            bool ok   = std::is_void_v<Ret> || julia_supported<std::remove_cvref_t<Ret>>();
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((ok = ok && julia_supported<
                                 std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>()),
                 ...);
            }(std::make_index_sequence<params.size()>{});
            return ok;
        }

        // A constructor is bindable only if every parameter type can cross
        // the boundary.
        template <std::meta::info Ctor> consteval bool ctor_supported() {
            bool           ok     = true;
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((ok = ok && julia_supported<
                                 std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>()),
                 ...);
            }(std::make_index_sequence<params.size()>{});
            return ok;
        }

    } // namespace julia_detail

    // ---- Visitor ----

    template <typename T> struct JuliaVisitor {
        jlcxx::Module         &mod;
        jlcxx::TypeWrapper<T> &cls;
        bool                   saw_default_ctor = false;

        template <std::meta::info Ctor, auto... /*Anns*/> void constructor() {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            if constexpr (params.size() == 0) {
                saw_default_ctor = true;
            }
            if constexpr (julia_detail::ctor_supported<Ctor>()) {
                register_ctor<Ctor>(std::make_index_sequence<params.size()>{});
            }
        }

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F = [:std::meta::type_of(Fld):];

            if constexpr (!julia_detail::julia_supported<std::remove_cvref_t<F>>()) {
                // Field type cannot cross the boundary — skip it.
            } else {
                // Getter: `name(obj) -> F`.
                cls.method(name, [](const T &s) -> F { return s.[:Fld:]; });

                // Mutating setter `name!(obj, v)` — omitted for read-only
                // fields, range-checked when a range annotation is present.
                if constexpr (ann::has<readonly>(Anns...)) {
                    // read-only: no setter is registered
                } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                    constexpr auto r       = ann::get_or<range>(range{0, 0}, Anns...);
                    const std::string setter = std::string(name) + "!";
                    cls.method(setter, [name](T &s, F v) {
                        double d = static_cast<double>(v);
                        if (d < r.min || d > r.max) {
                            throw std::out_of_range(std::string(name) + " out of range [" +
                                                    std::to_string(r.min) + ", " +
                                                    std::to_string(r.max) + "]");
                        }
                        s.[:Fld:] = v;
                    });
                } else {
                    const std::string setter = std::string(name) + "!";
                    cls.method(setter, [](T &s, F v) { s.[:Fld:] = v; });
                }
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_instance(const char *name) {
            if constexpr (julia_detail::method_supported<Fn>()) {
                // jlcxx makes the object the first Julia argument: `name(obj, ...)`.
                cls.method(name, &[:Fn:]);
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_static(const char *name) {
            if constexpr (julia_detail::method_supported<Fn>()) {
                // Julia has no static methods; expose as a module-level function.
                mod.method(name, &[:Fn:]);
            }
        }

      private:
        template <std::meta::info Ctor, std::size_t... Is>
        void register_ctor(std::index_sequence<Is...>) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            cls.template constructor<
                std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>...>();
        }
    };

    template <typename T> inline void bind_julia(jlcxx::Module &mod, const char *name) {
        jlcxx::TypeWrapper<T> cls = mod.add_type<T>(name);
        JuliaVisitor<T>       v{mod, cls};
        walk<T>(v);
        // The implicitly-declared default ctor may not be enumerated by
        // reflection; register one so `T()` keeps working from Julia. The
        // constructibility test must be `if constexpr`: a plain `if` still
        // instantiates `constructor<>()` (jlcxx's default-construct) for the
        // discarded branch, a hard error for a non-default-constructible type.
        if constexpr (std::is_default_constructible_v<T>) {
            if (!v.saw_default_ctor) {
                cls.template constructor<>();
            }
        }
    }

    template <typename T> inline void bind_julia_enum(jlcxx::Module &mod, const char *name) {
        mod.add_bits<T>(name, jlcxx::julia_type("CppEnum"));
        template for (constexpr auto en :
                      std::define_static_array(std::meta::enumerators_of(^^T))) {
            constexpr const char *nm = std::define_static_string(std::meta::identifier_of(en));
            mod.set_const(nm, [:en:]);
        }
    }

    template <std::meta::info F>
    inline void bind_julia_function(jlcxx::Module &mod, const char *name) {
        mod.method(name, &[:F:]);
    }

} // namespace rosetta
