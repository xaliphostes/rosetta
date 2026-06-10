namespace rosetta {

    template <typename T> struct PybindVisitor {
        py::class_<T> &cls;
        bool           saw_default_ctor = false;

        template <std::meta::info Ctor, auto... Anns> void constructor() {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            constexpr auto dann   = ann::get_or<doc>(doc{""}, Anns...);
            if constexpr (params.size() == 0)
                saw_default_ctor = true;
            register_init<Ctor>(std::make_index_sequence<params.size()>{}, dann.text);
        }

      private:
        template <std::meta::info Ctor, std::size_t... Is>
        void register_init(std::index_sequence<Is...>, const char *docstr) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            cls.def(py::init<typename[:std::meta::type_of(params[Is]):]...>(), docstr);
        }

      public:

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F                      = [:std::meta::type_of(Fld):];
            constexpr auto        dann   = ann::get_or<doc>(doc{""}, Anns...);
            constexpr const char *docstr = dann.text;

            if constexpr (ann::has<readonly>(Anns...)) {
                cls.def_property_readonly(name, [](const T &s) -> F { return s.[:Fld:]; }, docstr);
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                cls.def_property(
                    name, [](const T &s) -> F { return s.[:Fld:]; },
                    [name](T &s, F v) {
                        double d = static_cast<double>(v);
                        if (d < r.min || d > r.max) {
                            throw py::value_error(std::string(name) + " out of range [" +
                                                  std::to_string(r.min) + ", " +
                                                  std::to_string(r.max) + "]");
                        }
                        s.[:Fld:] = v;
                    },
                    docstr);
            } else {
                cls.def_property(
                    name, [](const T &s) -> F { return s.[:Fld:]; },
                    [](T &s, F v) { s.[:Fld:] = v; }, docstr);
            }
        }

        template <std::meta::info Fn, auto... Anns> void method_instance(const char *name) {
            constexpr auto dann = ann::get_or<doc>(doc{""}, Anns...);
            cls.def(name, &[:Fn:], dann.text);
        }

        template <std::meta::info Fn, auto... Anns> void method_static(const char *name) {
            constexpr auto dann = ann::get_or<doc>(doc{""}, Anns...);
            cls.def_static(name, &[:Fn:], dann.text);
        }
    };

    template <typename T> inline void bind_pybind(py::module_ &m, const char *py_name) {
        py::class_<T>    cls(m, py_name);
        PybindVisitor<T> v{cls};
        walk<T>(v);
        // The implicitly-declared default ctor may not be enumerated by
        // reflection; register one so `T()` keeps working.
        if (!v.saw_default_ctor && std::is_default_constructible_v<T>) {
            cls.def(py::init<>());
        }
    }

    template <typename T> inline void bind_pybind_enum(py::module_ &m, const char *py_name) {
        // No export_values(): enumerators are accessed as `Name.Value`, matching
        // scoped-enum semantics and avoiding polluting the module namespace
        // (an enumerator could otherwise collide with a bound class name).
        py::enum_<T> e(m, py_name);
        template for (constexpr auto en :
                      std::define_static_array(std::meta::enumerators_of(^^T))) {
            constexpr const char *nm = std::define_static_string(std::meta::identifier_of(en));
            e.value(nm, [:en:]);
        }
    }

} // namespace rosetta
