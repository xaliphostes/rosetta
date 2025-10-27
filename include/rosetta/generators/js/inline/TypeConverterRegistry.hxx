namespace rosetta::generators::js {

    inline TypeConverterRegistry::TypeConverterRegistry() {
        register_builtin_types();
    }

    inline TypeConverterRegistry &TypeConverterRegistry::instance() {
        static TypeConverterRegistry registry;
        return registry;
    }

    template <typename T>
    inline void
    TypeConverterRegistry::register_converter(std::unique_ptr<NapiTypeConverter<T>> converter) {
        converters_[std::type_index(typeid(T))] = std::move(converter);
    }

    template <typename T>
    inline void TypeConverterRegistry::register_class_wrapper(const std::string &class_name) {
        class_wrappers_.insert_or_assign(class_name, std::type_index(typeid(T)));
    }

    template <typename T>
    inline const NapiTypeConverter<T> *TypeConverterRegistry::get_converter() const {
        auto it = converters_.find(std::type_index(typeid(T)));
        if (it == converters_.end()) {
            return nullptr;
        }
        return static_cast<const NapiTypeConverter<T> *>(it->second.get());
    }

    template <typename T>
    inline Napi::Value TypeConverterRegistry::to_napi(Napi::Env env, const T &value) const {
        auto *converter = get_converter<T>();
        if (!converter) {
            throw NapiConversionError("No converter registered for type: " +
                                      std::string(typeid(T).name()));
        }
        return converter->to_napi_impl(env, value);
    }

    template <typename T>
    inline T TypeConverterRegistry::from_napi(Napi::Env env, Napi::Value js_value) const {
        auto *converter = get_converter<T>();
        if (!converter) {
            throw NapiConversionError("No converter registered for type: " +
                                      std::string(typeid(T).name()));
        }
        return converter->from_napi_impl(env, js_value);
    }

    template <typename T> inline bool TypeConverterRegistry::has_converter() const {
        return converters_.find(std::type_index(typeid(T))) != converters_.end();
    }

    template <typename Container>
    inline void TypeConverterRegistry::register_container_if_needed() {
        if (has_converter<Container>()) {
            return;
        }

        using traits::is_array_v;
        using traits::is_map_v;
        using traits::is_optional_v;
        using traits::is_vector_v;

        if constexpr (is_vector_v<Container>) {
            using T = typename traits::container_traits<Container>::value_type;
            register_converter<Container>(std::make_unique<VectorConverter<T>>());
        } else if constexpr (is_array_v<Container>) {
            using T            = typename traits::container_traits<Container>::value_type;
            constexpr size_t N = traits::container_traits<Container>::size;
            register_converter<Container>(std::make_unique<ArrayConverter<T, N>>());
        } else if constexpr (is_map_v<Container>) {
            using K = typename traits::container_traits<Container>::key_type;
            using V = typename traits::container_traits<Container>::value_type;
            register_converter<Container>(std::make_unique<MapConverter<K, V>>());
        } else if constexpr (is_optional_v<Container>) {
            using T = typename traits::container_traits<Container>::value_type;
            register_converter<Container>(std::make_unique<OptionalConverter<T>>());
        }
    }

    inline void TypeConverterRegistry::register_builtin_types() {
        register_converter<bool>(std::make_unique<BoolConverter>());
        register_converter<double>(std::make_unique<DoubleConverter>());
        register_converter<float>(std::make_unique<FloatConverter>());
        register_converter<int32_t>(std::make_unique<Int32Converter>());
        register_converter<uint32_t>(std::make_unique<UInt32Converter>());
        register_converter<int64_t>(std::make_unique<Int64Converter>());
        register_converter<std::string>(std::make_unique<StringConverter>());
    }

    // A generic factory that tries all registered class wrappers you stored (name → type_index)
    inline core::Any TypeConverterRegistry::to_any(Napi::Env env, Napi::Value js) const {
        if (js.IsBoolean())
            return core::Any(js.As<Napi::Boolean>().Value());
        if (js.IsNumber())
            return core::Any(js.As<Napi::Number>().DoubleValue());
        if (js.IsString())
            return core::Any(js.As<Napi::String>().Utf8Value());

        if (js.IsObject()) {
            // Use constructor name to decide which unwrap to attempt
            auto ctor  = js.As<Napi::Object>().Get("constructor").As<Napi::Function>();
            auto cname = ctor.Get("name").As<Napi::String>().Utf8Value();
            
            // auto it    = class_wrappers_.find(cname);
            // if (it != class_wrappers_.end()) {
            //     // Dispatch by known types. For a small set this is fine;
            //     // you can extend with if/else or a small registry of lambdas.
            //     if (cname == "Vector3D")
            //         return core::Any(unwrap_object_as<Vector3D>(env, js));
            //     // add other classes here…
            // }
        }

        return core::Any(); // undefined/null or unsupported
    }

} // namespace rosetta::generators::js