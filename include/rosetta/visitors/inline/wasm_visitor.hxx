namespace rosetta {

    template <typename T> struct EmscriptenVisitor {
        emscripten::class_<T> &cls;
        bool                   saw_default_ctor = false;

        template <std::meta::info Ctor, auto... /*Anns*/> void constructor() {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            if constexpr (params.size() == 0)
                saw_default_ctor = true;
            register_ctor<Ctor>(std::make_index_sequence<params.size()>{});
        }

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

      private:
        template <std::meta::info Ctor, std::size_t... Is>
        void register_ctor(std::index_sequence<Is...>) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            cls.template constructor<typename[:std::meta::type_of(params[Is]):]...>();
        }
    };

    template <typename T> void bind_wasm(const char *class_name) {
        emscripten::class_<T> cls(class_name);
        EmscriptenVisitor<T>  v{cls};
        walk<T>(v);
        // The implicitly-declared default ctor may not be enumerated by
        // reflection; register one so `new Module.T()` keeps working.
        if (!v.saw_default_ctor && std::is_default_constructible_v<T>)
            cls.template constructor<>();
    }

    template <typename T> void bind_wasm_enum(const char *enum_name) {
        emscripten::enum_<T> e(enum_name);
        template for (constexpr auto en :
                      std::define_static_array(std::meta::enumerators_of(^^T))) {
            constexpr const char *nm = std::define_static_string(std::meta::identifier_of(en));
            e.value(nm, [:en:]);
        }
    }

} // namespace rosetta
