namespace rosetta {

    // ---- Type classification ----

    template <typename T> struct is_std_vector : std::false_type {};
    template <typename U, typename A> struct is_std_vector<std::vector<U, A>> : std::true_type {};

    template <typename T> struct is_std_function : std::false_type {};
    template <typename R, typename... A>
    struct is_std_function<std::function<R(A...)>> : std::true_type {};

    // Can a value of T cross the N-API boundary? Scalars, strings, vectors of
    // supported types, and user class types (marshalled as wrapped objects)
    // are convertible; std::function and other unhandled types are not.
    template <typename T> consteval bool napi_supported() {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, bool> ||
                      std::is_arithmetic_v<U>) {
            return true;
        } else if constexpr (is_std_function<U>::value) {
            return false;
        } else if constexpr (is_std_vector<U>::value) {
            return napi_supported<typename U::value_type>();
        } else if constexpr (std::is_enum_v<U>) {
            return true; // marshalled as its underlying integer
        } else if constexpr (std::is_class_v<U>) {
            return true; // user type → wrapped Napi object
        } else {
            return false;
        }
    }

    // ---- Forward declarations (mutually recursive with conversions) ----

    template <typename T> class Wrap;
    template <typename T> Napi::FunctionReference &ctor_ref();

    // Constructor dispatch table for T, keyed by argument count. Populated by
    // bind_napi<T> (via the walk) and consulted by Wrap<T>'s constructor.
    // NOTE: dispatch is by arity only — two constructors with the same number
    // of parameters but different types cannot be told apart here.
    template <typename T>
    std::unordered_map<std::size_t, std::function<T(const Napi::CallbackInfo &)>> &ctor_table() {
        static std::unordered_map<std::size_t, std::function<T(const Napi::CallbackInfo &)>> table;
        return table;
    }

    // ---- Type conversion helpers ----

    template <typename T> Napi::Value to_napi(Napi::Env env, const T &v) {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<U, std::string>) {
            return Napi::String::New(env, v);
        } else if constexpr (std::is_same_v<U, bool>) {
            return Napi::Boolean::New(env, v);
        } else if constexpr (std::is_floating_point_v<U> || std::is_integral_v<U>) {
            return Napi::Number::New(env, static_cast<double>(v));
        } else if constexpr (is_std_vector<U>::value) {
            Napi::Array arr = Napi::Array::New(env, v.size());
            for (std::size_t i = 0; i < v.size(); ++i)
                arr.Set(static_cast<uint32_t>(i), to_napi(env, v[i]));
            return arr;
        } else if constexpr (std::is_enum_v<U>) {
            return Napi::Number::New(
                env, static_cast<double>(static_cast<std::underlying_type_t<U>>(v)));
        } else if constexpr (std::is_class_v<U>) {
            // User type: build a fresh wrapped object and copy the value in.
            Napi::Object obj = ctor_ref<U>().New({});
            Wrap<U>::Unwrap(obj)->inner = v;
            return obj;
        } else {
            static_assert(sizeof(T) == 0, "to_napi: unsupported type");
        }
    }

    template <typename T> T from_napi(const Napi::Value &v) {
        if constexpr (std::is_same_v<T, std::string>) {
            return v.As<Napi::String>().Utf8Value();
        } else if constexpr (std::is_same_v<T, bool>) {
            return v.As<Napi::Boolean>().Value();
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(v.As<Napi::Number>().DoubleValue());
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(v.As<Napi::Number>().Int64Value());
        } else if constexpr (is_std_vector<T>::value) {
            using Elem      = typename T::value_type;
            Napi::Array arr = v.As<Napi::Array>();
            T           out;
            out.reserve(arr.Length());
            for (uint32_t i = 0; i < arr.Length(); ++i)
                out.push_back(from_napi<Elem>(arr.Get(i)));
            return out;
        } else if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(
                static_cast<std::underlying_type_t<T>>(v.As<Napi::Number>().Int64Value()));
        } else if constexpr (std::is_class_v<T>) {
            // User type: unwrap the wrapped object and copy the value out.
            return Wrap<T>::Unwrap(v.As<Napi::Object>())->inner;
        } else {
            static_assert(sizeof(T) == 0, "from_napi: unsupported type");
        }
    }

    // ---- CRTP wrapper template ----

    template <typename T> class Wrap : public Napi::ObjectWrap<Wrap<T>> {
    public:
        T inner;

        Wrap(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Wrap<T>>(info) {
            // `inner` is default-constructed above; if the call arity matches a
            // registered constructor, rebuild it from the JS arguments.
            auto &tbl = ctor_table<T>();
            auto  it  = tbl.find(info.Length());
            if (it != tbl.end())
                inner = it->second(info);
            else if (info.Length() > 0)
                throw Napi::TypeError::New(info.Env(),
                                           "no matching constructor for " +
                                               std::to_string(info.Length()) + " argument(s)");
        }

        template <std::meta::info Fld>
        Napi::Value get_field(const Napi::CallbackInfo &info) {
            return to_napi(info.Env(), inner.[:Fld:]);
        }

        template <std::meta::info Fld>
        void set_field(const Napi::CallbackInfo &info, const Napi::Value &v) {
            using FieldT  = [:std::meta::type_of(Fld):];
            inner.[:Fld:] = from_napi<FieldT>(v);
        }

        template <std::meta::info Fld, rosetta::range R>
        void set_field_ranged(const Napi::CallbackInfo &info, const Napi::Value &v) {
            using FieldT        = [:std::meta::type_of(Fld):];
            constexpr auto name = std::define_static_string(std::meta::identifier_of(Fld));
            FieldT         val  = from_napi<FieldT>(v);
            double         d    = static_cast<double>(val);
            if (d < R.min || d > R.max) {
                throw Napi::RangeError::New(info.Env(), std::string(name) + " out of range");
            }
            inner.[:Fld:] = val;
        }

        template <std::meta::info Fld>
        void set_field_readonly(const Napi::CallbackInfo &info, const Napi::Value & /*v*/) {
            constexpr auto name = std::define_static_string(std::meta::identifier_of(Fld));
            throw Napi::TypeError::New(info.Env(), std::string(name) + " is read-only");
        }

        template <std::meta::info Fn>
        Napi::Value call_method(const Napi::CallbackInfo &info) {
            return call_method_impl<Fn>(
                info, std::make_index_sequence<
                          std::define_static_array(std::meta::parameters_of(Fn)).size()>{});
        }

        template <std::meta::info Fn>
        static Napi::Value call_static(const Napi::CallbackInfo &info) {
            return call_static_impl<Fn>(
                info, std::make_index_sequence<
                          std::define_static_array(std::meta::parameters_of(Fn)).size()>{});
        }

    private:
        template <std::meta::info Fn, std::size_t... Is>
        Napi::Value call_method_impl(const Napi::CallbackInfo &info,
                                     std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                (inner.[:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return info.Env().Undefined();
            } else {
                R r = (inner.[:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return to_napi(info.Env(), r);
            }
        }

        template <std::meta::info Fn, std::size_t... Is>
        static Napi::Value call_static_impl(const Napi::CallbackInfo &info,
                                            std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                ([:Fn:])(from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                    info[Is])...);
                return info.Env().Undefined();
            } else {
                R r = ([:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return to_napi(info.Env(), r);
            }
        }
    };

    // Per-type persistent constructor, set by bind_napi<T> and used by
    // to_napi to build fresh wrapped instances of user types.
    template <typename T> Napi::FunctionReference &ctor_ref() {
        static Napi::FunctionReference ref;
        return ref;
    }

    // ---- Member support predicate ----

    // A method is bindable only if its return type and every parameter type
    // can cross the N-API boundary; otherwise the walk skips it.
    template <std::meta::info Fn> consteval bool method_supported() {
        using Ret = [:std::meta::return_type_of(Fn):];
        bool ok   = std::is_void_v<Ret> || napi_supported<std::remove_cvref_t<Ret>>();
        constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((ok = ok &&
                   napi_supported<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>()),
             ...);
        }(std::make_index_sequence<params.size()>{});
        return ok;
    }

    // A constructor is bindable only if every parameter type can cross the
    // N-API boundary.
    template <std::meta::info Ctor> consteval bool ctor_supported() {
        bool           ok     = true;
        constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((ok = ok &&
                   napi_supported<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>()),
             ...);
        }(std::make_index_sequence<params.size()>{});
        return ok;
    }

    // ---- Visitor ----

    template <typename T> struct NapiVisitor {
        using This = Wrap<T>;
        std::vector<Napi::ClassPropertyDescriptor<This>> &props;

        template <std::meta::info Ctor, auto... /*Anns*/> void constructor() {
            if constexpr (ctor_supported<Ctor>()) {
                register_ctor<Ctor>(
                    std::make_index_sequence<
                        std::define_static_array(std::meta::parameters_of(Ctor)).size()>{});
            }
        }

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F = [:std::meta::type_of(Fld):];

            if constexpr (!napi_supported<std::remove_cvref_t<F>>()) {
                // Field type cannot cross the boundary — skip it.
            } else if constexpr (ann::has<readonly>(Anns...)) {
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field_readonly<Fld>>(
                        name));
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field_ranged<Fld, r>>(
                        name));
            } else {
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field<Fld>>(name));
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_instance(const char *name) {
            if constexpr (method_supported<Fn>()) {
                props.push_back(
                    This::template InstanceMethod<&This::template call_method<Fn>>(name));
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_static(const char *name) {
            if constexpr (method_supported<Fn>()) {
                props.push_back(
                    This::template StaticMethod<&This::template call_static<Fn>>(name));
            }
        }

      private:
        // Build a T from the JS arguments for a specific constructor. A named
        // static template (not a lambda) so the std::function stores a plain
        // function pointer — capturing the consteval-only param pack would be
        // ill-formed.
        template <std::meta::info Ctor, std::size_t... Is>
        static T construct(const Napi::CallbackInfo &info) {
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            return T(from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                info[Is])...);
        }

        template <std::meta::info Ctor, std::size_t... Is>
        void register_ctor(std::index_sequence<Is...>) {
            ctor_table<T>()[sizeof...(Is)] = &construct<Ctor, Is...>;
        }
    };

    template <typename T>
    inline Napi::Function bind_napi(Napi::Env env, const char *class_name) {
        using This = Wrap<T>;
        std::vector<Napi::ClassPropertyDescriptor<This>> props;
        NapiVisitor<T> v{props};
        walk<T>(v);
        Napi::Function ctor = This::DefineClass(env, class_name, props);
        // Persist the constructor so to_napi can build instances of T later.
        ctor_ref<T>() = Napi::Persistent(ctor);
        ctor_ref<T>().SuppressDestruct();
        return ctor;
    }

    template <typename T> inline Napi::Object bind_napi_enum(Napi::Env env) {
        Napi::Object obj = Napi::Object::New(env);
        template for (constexpr auto en :
                      std::define_static_array(std::meta::enumerators_of(^^T))) {
            constexpr const char *nm = std::define_static_string(std::meta::identifier_of(en));
            obj.Set(nm, Napi::Number::New(
                            env, static_cast<double>(static_cast<std::underlying_type_t<T>>(
                                     [:en:]))));
        }
        obj.Freeze();
        return obj;
    }

    // ---- Free function support ----

    template <std::meta::info F, std::size_t... Is>
    inline Napi::Value napi_free_call(const Napi::CallbackInfo &info, std::index_sequence<Is...>) {
        using R               = [:std::meta::return_type_of(F):];
        constexpr auto params = std::define_static_array(std::meta::parameters_of(F));
        if constexpr (std::is_void_v<R>) {
            ([:F:])(from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                info[Is])...);
            return info.Env().Undefined();
        } else {
            R r = ([:F:])(from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                info[Is])...);
            return to_napi(info.Env(), r);
        }
    }

    template <std::meta::info F> inline Napi::Value napi_free_entry(const Napi::CallbackInfo &info) {
        constexpr auto arity = std::define_static_array(std::meta::parameters_of(F)).size();
        return napi_free_call<F>(info, std::make_index_sequence<arity>{});
    }

    template <std::meta::info F>
    inline Napi::Function bind_napi_function(Napi::Env env, const char *name) {
        return Napi::Function::New(env, &napi_free_entry<F>, name);
    }

} // namespace rosetta
