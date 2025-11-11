namespace rosetta::js {

    // ============================================================================
    // Type Converters - Convert between C++ Any and Napi::Value
    // ============================================================================

    /**
     * @brief Convert C++ Any to JavaScript value
     */
    inline Napi::Value any_to_js(Napi::Env env, const core::Any &value) {
        if (!value.has_value()) {
            return env.Null();
        }

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) {
            return Napi::Number::New(env, value.as<int>());
        }
        if (type == std::type_index(typeid(double))) {
            return Napi::Number::New(env, value.as<double>());
        }
        if (type == std::type_index(typeid(float))) {
            return Napi::Number::New(env, value.as<float>());
        }
        if (type == std::type_index(typeid(bool))) {
            return Napi::Boolean::New(env, value.as<bool>());
        }
        if (type == std::type_index(typeid(std::string))) {
            return Napi::String::New(env, value.as<std::string>());
        }

        // Void return (method that returns nothing)
        if (type == std::type_index(typeid(void))) {
            return env.Undefined();
        }

        // For complex types, return a generic object or handle specially
        return env.Undefined();
    }

    /**
     * @brief Convert JavaScript value to C++ Any
     */
    inline core::Any js_to_any(const Napi::Value &js_val, std::type_index expected_type) {
        // Handle primitives
        if (expected_type == std::type_index(typeid(int))) {
            if (js_val.IsNumber()) {
                return core::Any(js_val.As<Napi::Number>().Int32Value());
            }
        }
        if (expected_type == std::type_index(typeid(double))) {
            if (js_val.IsNumber()) {
                return core::Any(js_val.As<Napi::Number>().DoubleValue());
            }
        }
        if (expected_type == std::type_index(typeid(float))) {
            if (js_val.IsNumber()) {
                return core::Any(static_cast<float>(js_val.As<Napi::Number>().DoubleValue()));
            }
        }
        if (expected_type == std::type_index(typeid(bool))) {
            if (js_val.IsBoolean()) {
                return core::Any(js_val.As<Napi::Boolean>().Value());
            }
        }
        if (expected_type == std::type_index(typeid(std::string))) {
            if (js_val.IsString()) {
                return core::Any(js_val.As<Napi::String>().Utf8Value());
            }
        }

        return core::Any(); // Empty
    }

    // ============================================================================
    // JavaScript Class Wrapper
    // ============================================================================

    /**
     * @brief Wraps a C++ object for JavaScript using Node-API
     * This is the ONLY wrapper needed, and it doesn't require the C++ class
     * to inherit from anything.
     */
    template <typename T> class JsClassWrapper : public Napi::ObjectWrap<JsClassWrapper<T>> {
    public:
        static Napi::FunctionReference constructor;

        // Static storage for field and method names (to avoid lambda captures)
        static std::vector<std::string> field_names_;
        static std::vector<std::string> method_names_;

        /**
         * @brief Initialize and export the class to JavaScript
         */
        static Napi::Object Init(Napi::Env env, Napi::Object exports, const std::string &js_name) {
            // Get metadata from registry
            const auto &meta = core::Registry::instance().get<T>();

            // std::cerr << "doing class " << meta.name() << std::endl;

            // Store field and method names in static storage
            field_names_  = meta.fields();
            method_names_ = meta.methods();

            // Build property descriptors for DefineClass
            std::vector<typename Napi::ObjectWrap<JsClassWrapper<T>>::PropertyDescriptor>
                properties;

            // We need to use InstanceValue or a different approach
            // InstanceAccessor requires knowing the specific method at compile time
            // So we'll define the class first, then add properties dynamically

            // Define the class with just the constructor
            Napi::Function func =
                Napi::ObjectWrap<JsClassWrapper<T>>::DefineClass(env, js_name.c_str(), properties);

            // Get the prototype to add properties
            Napi::Object prototype = func.Get("prototype").As<Napi::Object>();

            // Add field accessors using napi_define_properties directly
            for (size_t i = 0; i < field_names_.size(); ++i) {
                const std::string &field_name = field_names_[i];
                std::cerr << "  add field " << field_name << std::endl;

                // Create a property descriptor for this field
                napi_property_descriptor desc = {
                    field_name.c_str(), nullptr, nullptr,      FieldGetter,
                    FieldSetter,        nullptr, napi_default, reinterpret_cast<void *>(i)};

                napi_status status = napi_define_properties(env, prototype, 1, &desc);
                if (status != napi_ok) {
                    Napi::Error::New(env, "Failed to define property").ThrowAsJavaScriptException();
                }
            }

            // Add methods
            for (size_t i = 0; i < method_names_.size(); ++i) {
                const std::string &method_name = method_names_[i];
                std::cerr << "  add method " << method_name << std::endl;

                napi_property_descriptor desc = {method_name.c_str(),
                                                 nullptr,
                                                 MethodCaller,
                                                 nullptr,
                                                 nullptr,
                                                 nullptr,
                                                 napi_default,
                                                 reinterpret_cast<void *>(i)};

                napi_status status = napi_define_properties(env, prototype, 1, &desc);
                if (status != napi_ok) {
                    Napi::Error::New(env, "Failed to define method").ThrowAsJavaScriptException();
                }
            }

            constructor = Napi::Persistent(func);
            constructor.SuppressDestruct();

            exports.Set(js_name, func);
            return exports;
        }

        /**
         * @brief Constructor from JavaScript
         */
        JsClassWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<JsClassWrapper<T>>(info) {

            Napi::Env   env   = info.Env();
            const auto &meta  = core::Registry::instance().get<T>();
            const auto &ctors = meta.constructors();

            // Find matching constructor by argument count
            for (const auto &ctor : ctors) {
                // Try to call with matching argument count
                // For simplicity, we'll just try default constructor first
                if (info.Length() == 0) {
                    try {
                        std::vector<core::Any> args;
                        core::Any              result = ctor(args);
                        cpp_object_                   = result.as<T>();
                        return;
                    } catch (...) {
                        continue;
                    }
                }
            }

            // Fallback: default construct if possible
            if constexpr (std::is_default_constructible_v<T>) {
                cpp_object_ = T();
            } else {
                Napi::Error::New(env, "No matching constructor found").ThrowAsJavaScriptException();
            }
        }

        /**
         * @brief Get the underlying C++ object
         */
        T       &GetCppObject() { return cpp_object_; }
        const T &GetCppObject() const { return cpp_object_; }

    private:
        T cpp_object_;

        /**
         * @brief C-style getter callback for fields (raw napi)
         */
        static napi_value FieldGetter(napi_env env, napi_callback_info info) {
            size_t     argc = 0;
            napi_value this_arg;
            void      *data;

            napi_get_cb_info(env, info, &argc, nullptr, &this_arg, &data);

            size_t             field_index = reinterpret_cast<size_t>(data);
            const std::string &field_name  = field_names_[field_index];

            // Unwrap the C++ object
            void *instance_data;
            napi_unwrap(env, this_arg, &instance_data);
            auto *wrapper = static_cast<JsClassWrapper<T> *>(instance_data);

            try {
                Napi::Env   napi_env(env);
                Napi::Value result = wrapper->GetField(napi_env, field_name);
                return result;
            } catch (const std::exception &e) {
                napi_throw_error(env, nullptr, e.what());
                return nullptr;
            }
        }

        /**
         * @brief C-style setter callback for fields (raw napi)
         */
        static napi_value FieldSetter(napi_env env, napi_callback_info info) {
            size_t     argc = 1;
            napi_value args[1];
            napi_value this_arg;
            void      *data;

            napi_get_cb_info(env, info, &argc, args, &this_arg, &data);

            if (argc < 1) {
                napi_throw_error(env, nullptr, "Setter requires a value");
                return nullptr;
            }

            size_t             field_index = reinterpret_cast<size_t>(data);
            const std::string &field_name  = field_names_[field_index];

            // Unwrap the C++ object
            void *instance_data;
            napi_unwrap(env, this_arg, &instance_data);
            auto *wrapper = static_cast<JsClassWrapper<T> *>(instance_data);

            try {
                Napi::Env   napi_env(env);
                Napi::Value value(napi_env, args[0]);
                wrapper->SetField(napi_env, field_name, value);
                return nullptr;
            } catch (const std::exception &e) {
                napi_throw_error(env, nullptr, e.what());
                return nullptr;
            }
        }

        /**
         * @brief C-style method callback (raw napi)
         */
        static napi_value MethodCaller(napi_env env, napi_callback_info info) {
            size_t     argc = 10; // Max args we'll accept
            napi_value args[10];
            napi_value this_arg;
            void      *data;

            napi_get_cb_info(env, info, &argc, args, &this_arg, &data);

            size_t             method_index = reinterpret_cast<size_t>(data);
            const std::string &method_name  = method_names_[method_index];

            // Unwrap the C++ object
            void *instance_data;
            napi_unwrap(env, this_arg, &instance_data);
            auto *wrapper = static_cast<JsClassWrapper<T> *>(instance_data);

            try {
                Napi::Env napi_env(env);

                // Build CallbackInfo
                std::vector<napi_value> arg_vec(args, args + argc);
                Napi::CallbackInfo      callback_info(napi_env, info);

                Napi::Value result = wrapper->CallMethod(napi_env, method_name, callback_info);
                return result;
            } catch (const std::exception &e) {
                napi_throw_error(env, nullptr, e.what());
                return nullptr;
            }
        }

        /**
         * @brief Property getter callback
         */
        Napi::Value GetField(Napi::Env env, const std::string &field_name) {
            try {
                const auto &meta  = core::Registry::instance().get<T>();
                core::Any   value = meta.get_field(cpp_object_, field_name);
                return any_to_js(env, value);
            } catch (const std::exception &e) {
                Napi::Error::New(env, std::string("Failed to get field: ") + e.what())
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }
        }

        /**
         * @brief Property setter callback
         */
        void SetField(Napi::Env env, const std::string &field_name, const Napi::Value &value) {
            try {
                const auto     &meta       = core::Registry::instance().get<T>();
                std::type_index field_type = meta.get_field_type(field_name);
                core::Any       cpp_value  = js_to_any(value, field_type);
                meta.set_field(cpp_object_, field_name, cpp_value);
            } catch (const std::exception &e) {
                Napi::Error::New(env, std::string("Failed to set field: ") + e.what())
                    .ThrowAsJavaScriptException();
            }
        }

        /**
         * @brief Method call callback
         */
        Napi::Value CallMethod(Napi::Env env, const std::string &method_name,
                               const Napi::CallbackInfo &info) {
            try {
                const auto &meta = core::Registry::instance().get<T>();

                // Get method argument types
                const auto &arg_types = meta.get_method_arg_types(method_name);

                // Convert JavaScript arguments to C++ Any
                std::vector<core::Any> args;
                for (size_t i = 0; i < info.Length() && i < arg_types.size(); ++i) {
                    args.push_back(js_to_any(info[i], arg_types[i]));
                }

                // Invoke the method
                core::Any result = meta.invoke_method(cpp_object_, method_name, args);

                // Convert result back to JavaScript
                return any_to_js(env, result);
            } catch (const std::exception &e) {
                Napi::Error::New(env, std::string("Method call failed: ") + e.what())
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }
        }
    };

    // Static member initialization
    template <typename T> Napi::FunctionReference JsClassWrapper<T>::constructor;

    template <typename T> std::vector<std::string> JsClassWrapper<T>::field_names_;

    template <typename T> std::vector<std::string> JsClassWrapper<T>::method_names_;

    // ============================================================================
    // JavaScript Binding Generator
    // ============================================================================

    inline JsGenerator::JsGenerator(Napi::Env env, Napi::Object exports)
        : env_(env), exports_(exports) {
    }

    template <typename T>
    inline JsGenerator &JsGenerator::bind_class(const std::string &js_name = "") {
        // Get metadata from registry
        const auto &meta       = core::Registry::instance().get<T>();
        std::string final_name = js_name.empty() ? meta.name() : js_name;

        // Initialize the wrapper
        JsClassWrapper<T>::Init(env_, exports_, final_name);

        return *this;
    }

    inline JsGenerator &JsGenerator::add_utilities() {
        // List all registered classes
        exports_.Set("listClasses", Napi::Function::New(env_, [](const Napi::CallbackInfo &info) {
                         auto        env     = info.Env();
                         const auto &classes = core::Registry::instance().list_classes();
                         auto        arr     = Napi::Array::New(env, classes.size());
                         for (size_t i = 0; i < classes.size(); ++i) {
                             arr.Set(i, Napi::String::New(env, classes[i]));
                         }
                         return arr;
                     }));

        return *this;
    }

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> inline void bind_classes(JsGenerator &gen) {
        (gen.bind_class<Classes>(), ...);
    }

} // namespace rosetta::js