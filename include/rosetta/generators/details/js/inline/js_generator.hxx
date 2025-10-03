/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#include <rosetta/introspectable.h>

namespace rosetta {

    /**
     * @brief Simple wrapper using Napi::ObjectWrap properly
     */
    template <typename T> class ObjectWrapper : public Napi::ObjectWrap<ObjectWrapper<T>> {
    public:
        static Napi::FunctionReference constructor;
        static Napi::Object Init(Napi::Env, Napi::Object, const std::string&);

        ObjectWrapper(const Napi::CallbackInfo& info);

        T* GetCppObject();

        /**
         * @brief Set this wrapper to reference an existing C++ object (non-owning)
         * @param ptr Pointer to existing C++ object
         */
        void SetNonOwningPointer(T* ptr)
        {
            if (!ptr) {
                cpp_obj = nullptr;
                return;
            }

            // Create non-owning shared_ptr with no-op deleter
            cpp_obj = std::shared_ptr<T>(ptr, [](T*) {
                // Do nothing - we don't own this pointer
            });

            // Setup all bindings for this object
            SetupBindings();
        }

    private:
        std::shared_ptr<T> cpp_obj;

        void SetupBindings();
        void SetupProperty(const std::string& prop_name);
        void SetupMethod(const std::string& method_name);
        void SetupIntrospection();
        static bool IsSimpleGetterSetter(const std::string&, const TypeInfo&);
        static std::string Capitalize(const std::string& str);
    };

    // ------------------------------------------------

    /**
     * @brief Type converters registry - singleton pattern
     */
    class TypeConverterRegistry {
    public:
        static TypeConverterRegistry& instance();

        void register_converter(const std::string&, CppToJsConverter, JsToCppConverter);
        Napi::Value convert_to_js(Napi::Env, const std::any&, const std::string&) const;
        std::any convert_to_cpp(const Napi::Value&, const std::string&) const;

    private:
        TypeConverterRegistry();
        std::unordered_map<std::string, CppToJsConverter> cpp_to_js_converters;
        std::unordered_map<std::string, JsToCppConverter> js_to_cpp_converters;
    };

    // ------------------------------------------------

    template <typename T>
    inline Napi::Object ObjectWrapper<T>::Init(
        Napi::Env env, Napi::Object exports, const std::string& class_name)
    {
        Napi::Function func = ObjectWrapper::DefineClass(env, class_name.c_str(),
            {
                // These are just placeholders - real
                // methods are added in constructor
            });

        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();

        exports.Set(class_name, func);
        return exports;
    }

    template <typename T>
    inline ObjectWrapper<T>::ObjectWrapper(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<ObjectWrapper<T>>(info)
    {

        auto env = info.Env();

        // Get type info to access constructors
        const auto& type_info = T::getStaticTypeInfo();
        const auto& ctors = type_info.getConstructors();

        // Find matching constructor based on argument count
        const ConstructorInfo* matching_ctor = nullptr;
        for (const auto& ctor : ctors) {
            if (ctor->parameter_types.size() == info.Length()) {
                matching_ctor = ctor.get();
                break;
            }
        }

        if (!matching_ctor) {
            // Default constructor as fallback
            cpp_obj = std::make_shared<T>();
        } else {
            try {
                // Convert JS arguments to C++ std::any
                std::vector<std::any> args;
                for (size_t i = 0; i < info.Length(); ++i) {
                    args.push_back(TypeConverterRegistry::instance().convert_to_cpp(
                        info[i], matching_ctor->parameter_types[i]));
                }

                // Create object using factory and transfer ownership
                void* raw_ptr = matching_ctor->factory(args);
                cpp_obj = std::shared_ptr<T>(static_cast<T*>(raw_ptr));
            } catch (const std::exception& e) {
                Napi::Error::New(env, std::string("Constructor failed: ") + e.what())
                    .ThrowAsJavaScriptException();
                cpp_obj = std::make_shared<T>(); // Fallback
            }
        }

        // Setup all bindings
        SetupBindings();
    }

    template <typename T> inline T* ObjectWrapper<T>::GetCppObject() { return cpp_obj.get(); }

    template <typename T> inline void ObjectWrapper<T>::SetupBindings()
    {
        // auto env = this->Env();
        // auto obj = this->Value();
        const auto& type_info = cpp_obj->getTypeInfo();

        // Bind properties
        for (const auto& member_name : type_info.getMemberNames()) {
            SetupProperty(member_name);
        }

        // Bind methods
        for (const auto& method_name : type_info.getMethodNames()) {
            // Skip ONLY if it's a simple getter/setter for an existing member
            if (!IsSimpleGetterSetter(method_name, type_info)) {
                SetupMethod(method_name);
            }
        }

        // Bind introspection
        SetupIntrospection();
    }

    template <typename T> inline void ObjectWrapper<T>::SetupProperty(const std::string& prop_name)
    {
        auto env = this->Env();
        auto obj = this->Value();

        // Get Object.defineProperty
        auto global = env.Global();
        auto object_ctor = global.Get("Object").template As<Napi::Object>();
        auto define_prop = object_ctor.Get("defineProperty").template As<Napi::Function>();

        // Create descriptor
        auto descriptor = Napi::Object::New(env);
        descriptor.Set("enumerable", true);
        descriptor.Set("configurable", true);

        // Getter
        descriptor.Set("get",
            Napi::Function::New(
                env, [this, prop_name](const Napi::CallbackInfo& info) -> Napi::Value {
                    try {
                        auto val = cpp_obj->getMemberValue(prop_name);
                        const auto* mem = cpp_obj->getTypeInfo().getMember(prop_name);
                        return TypeConverterRegistry::instance().convert_to_js(
                            info.Env(), val, mem->type_name);
                    } catch (const std::exception& e) {
                        Napi::Error::New(info.Env(), e.what()).ThrowAsJavaScriptException();
                        return info.Env().Undefined();
                    }
                }));

        // Setter
        descriptor.Set("set",
            Napi::Function::New(
                env, [this, prop_name](const Napi::CallbackInfo& info) -> Napi::Value {
                    try {
                        if (info.Length() < 1) {
                            Napi::TypeError::New(info.Env(), "Expected 1 argument")
                                .ThrowAsJavaScriptException();
                            return info.Env().Undefined();
                        }
                        const auto* mem = cpp_obj->getTypeInfo().getMember(prop_name);
                        auto cpp_val = TypeConverterRegistry::instance().convert_to_cpp(
                            info[0], mem->type_name);
                        cpp_obj->setMemberValue(prop_name, cpp_val);
                        return info.Env().Undefined();
                    } catch (const std::exception& e) {
                        Napi::Error::New(info.Env(), e.what()).ThrowAsJavaScriptException();
                        return info.Env().Undefined();
                    }
                }));

        // Define the property
        define_prop.Call({ obj, Napi::String::New(env, prop_name), descriptor });

        // Also add explicit getter/setter methods
        std::string getter = "get" + Capitalize(prop_name);
        std::string setter = "set" + Capitalize(prop_name);

        obj.Set(getter, Napi::Function::New(env, [this, prop_name](const Napi::CallbackInfo& info) {
            auto val = cpp_obj->getMemberValue(prop_name);
            const auto* mem = cpp_obj->getTypeInfo().getMember(prop_name);
            return TypeConverterRegistry::instance().convert_to_js(info.Env(), val, mem->type_name);
        }));

        obj.Set(setter, Napi::Function::New(env, [this, prop_name](const Napi::CallbackInfo& info) {
            if (info.Length() >= 1) {
                const auto* mem = cpp_obj->getTypeInfo().getMember(prop_name);
                auto cpp_val
                    = TypeConverterRegistry::instance().convert_to_cpp(info[0], mem->type_name);
                cpp_obj->setMemberValue(prop_name, cpp_val);
            }
            return info.Env().Undefined();
        }));
    }

    template <typename T> inline void ObjectWrapper<T>::SetupMethod(const std::string& method_name)
    {
        auto env = this->Env();
        auto obj = this->Value();

        obj.Set(method_name,
            Napi::Function::New(env, [this, method_name](const Napi::CallbackInfo& info) {
                try {
                    const auto* meth = cpp_obj->getTypeInfo().getMethod(method_name);

                    if (info.Length() != meth->parameter_types.size()) {
                        std::string err = "Expected " + std::to_string(meth->parameter_types.size())
                            + " arguments, got " + std::to_string(info.Length());
                        Napi::TypeError::New(info.Env(), err).ThrowAsJavaScriptException();
                        return info.Env().Undefined();
                    }

                    std::vector<std::any> args;
                    for (size_t i = 0; i < info.Length(); ++i) {
                        args.push_back(TypeConverterRegistry::instance().convert_to_cpp(
                            info[i], meth->parameter_types[i]));
                    }

                    auto result = cpp_obj->callMethod(method_name, args);
                    return TypeConverterRegistry::instance().convert_to_js(
                        info.Env(), result, meth->return_type);

                } catch (const std::exception& e) {
                    Napi::Error::New(info.Env(), e.what()).ThrowAsJavaScriptException();
                    return info.Env().Undefined();
                }
            }));
    }

    template <typename T> inline void ObjectWrapper<T>::SetupIntrospection()
    {
        auto env = this->Env();
        auto obj = this->Value();

        obj.Set("getClassName", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            return Napi::String::New(info.Env(), cpp_obj->getClassName());
        }));

        obj.Set("getMemberNames", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            auto names = cpp_obj->getMemberNames();
            auto arr = Napi::Array::New(info.Env());
            for (size_t i = 0; i < names.size(); ++i) {
                arr.Set(i, names[i]);
            }
            return arr;
        }));

        obj.Set("getMethodNames", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            auto names = cpp_obj->getMethodNames();
            auto arr = Napi::Array::New(info.Env());
            for (size_t i = 0; i < names.size(); ++i) {
                arr.Set(i, names[i]);
            }
            return arr;
        }));

        obj.Set("hasMember", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            if (info.Length() > 0 && info[0].IsString()) {
                std::string name = info[0].template As<Napi::String>().Utf8Value();
                return Napi::Boolean::New(info.Env(), cpp_obj->hasMember(name));
            }
            return Napi::Boolean::New(info.Env(), false);
        }));

        obj.Set("hasMethod", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            if (info.Length() > 0 && info[0].IsString()) {
                std::string name = info[0].template As<Napi::String>().Utf8Value();
                return Napi::Boolean::New(info.Env(), cpp_obj->hasMethod(name));
            }
            return Napi::Boolean::New(info.Env(), false);
        }));

        obj.Set("toJSON", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            return Napi::String::New(info.Env(), cpp_obj->toJSON());
        }));

        obj.Set("getMemberValue", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            if (info.Length() > 0 && info[0].IsString()) {
                std::string name = info[0].template As<Napi::String>().Utf8Value();
                auto val = cpp_obj->getMemberValue(name);
                const auto* mem = cpp_obj->getTypeInfo().getMember(name);
                return TypeConverterRegistry::instance().convert_to_js(
                    info.Env(), val, mem ? mem->type_name : "unknown");
            }
            return info.Env().Undefined();
        }));

        obj.Set("setMemberValue", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            if (info.Length() >= 2 && info[0].IsString()) {
                std::string name = info[0].template As<Napi::String>().Utf8Value();
                const auto* mem = cpp_obj->getTypeInfo().getMember(name);
                if (mem) {
                    auto cpp_val
                        = TypeConverterRegistry::instance().convert_to_cpp(info[1], mem->type_name);
                    cpp_obj->setMemberValue(name, cpp_val);
                }
            }
            return info.Env().Undefined();
        }));

        obj.Set("callMethod", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
            if (info.Length() > 0 && info[0].IsString()) {
                std::string name = info[0].template As<Napi::String>().Utf8Value();
                const auto* meth = cpp_obj->getTypeInfo().getMethod(name);
                if (meth) {
                    std::vector<std::any> args;
                    if (info.Length() > 1 && info[1].IsArray()) {
                        auto arr = info[1].template As<Napi::Array>();
                        for (uint32_t i = 0; i < arr.Length() && i < meth->parameter_types.size();
                            ++i) {
                            args.push_back(TypeConverterRegistry::instance().convert_to_cpp(
                                arr.Get(i), meth->parameter_types[i]));
                        }
                    }
                    auto result = cpp_obj->callMethod(name, args);
                    return TypeConverterRegistry::instance().convert_to_js(
                        info.Env(), result, meth->return_type);
                }
            }
            return info.Env().Undefined();
        }));
    }

    template <typename T>
    inline bool ObjectWrapper<T>::IsSimpleGetterSetter(
        const std::string& method_name, const TypeInfo& type_info)
    {
        // Check if it's a getter: getName, getAge, etc.
        if (method_name.starts_with("get") && method_name.length() > 3) {
            std::string potential_member = method_name.substr(3);
            // Make first char lowercase
            potential_member[0] = std::tolower(potential_member[0]);

            // Only skip if this member actually exists
            if (type_info.getMember(potential_member) != nullptr) {
                return true;
            }
        }

        // Check if it's a setter: setName, setAge, etc.
        if (method_name.starts_with("set") && method_name.length() > 3) {
            std::string potential_member = method_name.substr(3);
            // Make first char lowercase
            potential_member[0] = std::tolower(potential_member[0]);

            // Only skip if this member actually exists
            if (type_info.getMember(potential_member) != nullptr) {
                return true;
            }
        }

        // Check if it's an "is" getter: isActive, isRunning, etc.
        if (method_name.starts_with("is") && method_name.length() > 2) {
            std::string potential_member = method_name.substr(2);
            // Make first char lowercase
            potential_member[0] = std::tolower(potential_member[0]);

            // Only skip if this member actually exists
            if (type_info.getMember(potential_member) != nullptr) {
                return true;
            }
        }

        return false;
    }

    template <typename T> inline std::string ObjectWrapper<T>::Capitalize(const std::string& str)
    {
        if (str.empty())
            return str;
        std::string result = str;
        result[0] = std::toupper(result[0]);
        return result;
    }

    // ================================================

    // Static member definition
    template <typename T> Napi::FunctionReference ObjectWrapper<T>::constructor;

    // ================================================================================================

    inline TypeConverterRegistry& TypeConverterRegistry::instance()
    {
        static TypeConverterRegistry registry;
        return registry;
    }

    inline void TypeConverterRegistry::register_converter(
        const std::string& type_name, CppToJsConverter to_js, JsToCppConverter to_cpp)
    {
        cpp_to_js_converters[type_name] = to_js;
        js_to_cpp_converters[type_name] = to_cpp;
    }

    inline Napi::Value TypeConverterRegistry::convert_to_js(
        Napi::Env env, const std::any& value, const std::string& type_name) const
    {
        if (!value.has_value() || type_name == "void") {
            return env.Undefined();
        }

        auto it = cpp_to_js_converters.find(type_name);
        if (it != cpp_to_js_converters.end()) {
            return it->second(env, value);
        }

        try {
            if (type_name == "string") {
                return Napi::String::New(env, std::any_cast<std::string>(value));
            } else if (type_name == "int") {
                return Napi::Number::New(env, std::any_cast<int>(value));
            } else if (type_name == "double") {
                return Napi::Number::New(env, std::any_cast<double>(value));
            } else if (type_name == "float") {
                return Napi::Number::New(env, std::any_cast<float>(value));
            } else if (type_name == "bool") {
                return Napi::Boolean::New(env, std::any_cast<bool>(value));
            }
        } catch (const std::bad_any_cast&) {
            // Fall through
        }

        return env.Undefined();
    }

    inline std::any TypeConverterRegistry::convert_to_cpp(
        const Napi::Value& js_value, const std::string& type_name) const
    {
        auto it = js_to_cpp_converters.find(type_name);
        if (it != js_to_cpp_converters.end()) {
            return it->second(js_value);
        }

        if (type_name == "string") {
            return std::make_any<std::string>(js_value.IsString()
                    ? js_value.As<Napi::String>().Utf8Value()
                    : js_value.ToString().Utf8Value());
        } else if (type_name == "int") {
            return std::make_any<int>(js_value.As<Napi::Number>().Int32Value());
        } else if (type_name == "double") {
            return std::make_any<double>(js_value.As<Napi::Number>().DoubleValue());
        } else if (type_name == "float") {
            return std::make_any<float>(
                static_cast<float>(js_value.As<Napi::Number>().DoubleValue()));
        } else if (type_name == "bool") {
            return std::make_any<bool>(js_value.As<Napi::Boolean>().Value());
        }

        throw std::runtime_error("Unsupported type: " + type_name);
    }

    inline TypeConverterRegistry::TypeConverterRegistry()
    {
        // Register vector converters
        register_converter(
            "vector<int>",
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                auto vec = std::any_cast<std::vector<int>>(value);
                auto arr = Napi::Array::New(env, vec.size());
                for (size_t i = 0; i < vec.size(); ++i) {
                    arr.Set(i, vec[i]);
                }
                return arr;
            },
            [](const Napi::Value& js_val) -> std::any {
                auto arr = js_val.As<Napi::Array>();
                std::vector<int> vec;
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    vec.push_back(arr.Get(i).As<Napi::Number>().Int32Value());
                }
                return vec;
            });

        register_converter(
            "vector<double>",
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                auto vec = std::any_cast<std::vector<double>>(value);
                auto arr = Napi::Array::New(env, vec.size());
                for (size_t i = 0; i < vec.size(); ++i) {
                    arr.Set(i, vec[i]);
                }
                return arr;
            },
            [](const Napi::Value& js_val) -> std::any {
                auto arr = js_val.As<Napi::Array>();
                std::vector<double> vec;
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    vec.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
                return vec;
            });

        register_converter(
            "vector<string>",
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                auto vec = std::any_cast<std::vector<std::string>>(value);
                auto arr = Napi::Array::New(env, vec.size());
                for (size_t i = 0; i < vec.size(); ++i) {
                    arr.Set(i, vec[i]);
                }
                return arr;
            },
            [](const Napi::Value& js_val) -> std::any {
                auto arr = js_val.As<Napi::Array>();
                std::vector<std::string> vec;
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    vec.push_back(arr.Get(i).As<Napi::String>().Utf8Value());
                }
                return vec;
            });
    }

    // ============================================================

    inline JsGenerator::JsGenerator(Napi::Env env, Napi::Object exports)
        : env(env)
        , exports(exports)
    {
    }

    inline JsGenerator& JsGenerator::add_utilities()
    {
        exports.Set(
            "getAllClasses", Napi::Function::New(env, [this](const Napi::CallbackInfo& info) {
                auto arr = Napi::Array::New(env);
                uint32_t idx = 0;
                for (const auto& name : bound_classes) {
                    arr.Set(idx++, name);
                }
                return arr;
            }));

        return *this;
    }

    inline JsGenerator& JsGenerator::register_type_converter(
        const std::string& type_name, CppToJsConverter to_js, JsToCppConverter to_cpp)
    {
        TypeConverterRegistry::instance().register_converter(type_name, to_js, to_cpp);
        return *this;
    }

    template <typename T> inline JsGenerator& JsGenerator::bind_class(const std::string& class_name)
    {
        static_assert(
            std::is_base_of_v<Introspectable, T>, "Type must inherit from Introspectable");

        const auto& type_info = T::getStaticTypeInfo();
        std::string final_name = class_name.empty() ? type_info.class_name : class_name;

        if (bound_classes.find(final_name) != bound_classes.end()) {
            throw std::runtime_error("Class already bound: " + final_name);
        }
        bound_classes.insert(final_name);

        ObjectWrapper<T>::Init(env, exports, final_name);
        return *this;
    }

    template <typename... Classes> inline JsGenerator& JsGenerator::bind_classes()
    {
        (bind_class<Classes>(), ...);
        return *this;
    }

    template <typename T> inline void bind_class(JsGenerator& generator)
    {
        generator.bind_class<T>();
    }

    template <typename... Classes> inline void bind_classes(JsGenerator& generator)
    {
        generator.bind_classes<Classes...>();
    }

}