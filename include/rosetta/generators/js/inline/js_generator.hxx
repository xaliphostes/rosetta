// ============================================================================
// Implementation of JsGenerator template methods with TypeInfo system
// ============================================================================
#pragma once

#include "../../../rosetta.h"
#include <sstream>
#include <stdexcept>

namespace rosetta::generators::js {

    // ========================================================================
    // WRAPPED OBJECT CLASS
    // ========================================================================

    /**
     * @brief Wrapper to hold C++ objects in JavaScript
     * This is stored as external data in JS objects
     */
    template <typename Class> struct WrappedObject {
        std::shared_ptr<Class> instance;
        TypeInfo               type_info;
        JsGenerator           *generator; // Pointer to generator for converter access

        explicit WrappedObject(Class *ptr, JsGenerator *gen = nullptr)
            : instance(ptr), type_info(TypeInfo::create<Class>()), generator(gen) {}
        explicit WrappedObject(std::shared_ptr<Class> ptr, JsGenerator *gen = nullptr)
            : instance(std::move(ptr)), type_info(TypeInfo::create<Class>()), generator(gen) {}
        
        ~WrappedObject() {
            // Shared pointer will handle cleanup automatically
        }
    };

    // ========================================================================
    // JSGENERATOR IMPLEMENTATION
    // ========================================================================

    inline JsGenerator::JsGenerator(Napi::Env env, Napi::Object exports)
        : env(env), exports(exports) {
        init_default_converters();
    }

    inline void JsGenerator::init_default_converters() {
        // Register basic types in TypeRegistry
        TypeRegistry::instance().register_type<int>("int");
        TypeRegistry::instance().register_type<double>("double");
        TypeRegistry::instance().register_type<float>("float");
        TypeRegistry::instance().register_type<bool>("bool");
        TypeRegistry::instance().register_type<std::string>("string");

        // int
        register_converter<int>(
            [](Napi::Env env, const core::Any &val) {
                return Napi::Number::New(env, val.as<int>());
            },
            [](const Napi::Value &val) { return core::Any(val.As<Napi::Number>().Int32Value()); });

        // double
        register_converter<double>(
            [](Napi::Env env, const core::Any &val) {
                return Napi::Number::New(env, val.as<double>());
            },
            [](const Napi::Value &val) { return core::Any(val.As<Napi::Number>().DoubleValue()); });

        // float
        register_converter<float>(
            [](Napi::Env env, const core::Any &val) {
                return Napi::Number::New(env, val.as<float>());
            },
            [](const Napi::Value &val) { return core::Any(val.As<Napi::Number>().FloatValue()); });

        // bool
        register_converter<bool>(
            [](Napi::Env env, const core::Any &val) {
                return Napi::Boolean::New(env, val.as<bool>());
            },
            [](const Napi::Value &val) { return core::Any(val.As<Napi::Boolean>().Value()); });

        // std::string
        register_converter<std::string>(
            [](Napi::Env env, const core::Any &val) {
                return Napi::String::New(env, val.as<std::string>());
            },
            [](const Napi::Value &val) { return core::Any(val.As<Napi::String>().Utf8Value()); });
    }

    template <typename T>
    inline JsGenerator &JsGenerator::register_converter(CppToJsConverter to_js,
                                                        JsToCppConverter from_js) {

        // Register type in TypeRegistry if not already registered
        if (!TypeRegistry::instance().has_type<T>()) {
            TypeRegistry::instance().register_type<T>();
        }

        std::type_index idx(typeid(T));

        // DEBUG: Print registration
        // std::cerr << "[DEBUG] Registering converter for type: " << idx.name() << " (T=" <<
        // typeid(T).name() << ")" << std::endl;

        cpp_to_js_[idx] = std::move(to_js);
        js_to_cpp_[idx] = std::move(from_js);
        return *this;
    }

    inline JsGenerator &JsGenerator::register_converter(std::type_index  type_idx,
                                                        CppToJsConverter to_js,
                                                        JsToCppConverter from_js) {

        cpp_to_js_[type_idx] = std::move(to_js);
        js_to_cpp_[type_idx] = std::move(from_js);
        return *this;
    }

    inline CppToJsConverter JsGenerator::get_to_js_converter(std::type_index idx) const {
        auto it = cpp_to_js_.find(idx);
        if (it != cpp_to_js_.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline JsToCppConverter JsGenerator::get_from_js_converter(std::type_index idx) const {
        auto it = js_to_cpp_.find(idx);
        if (it != js_to_cpp_.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline Napi::Value JsGenerator::any_to_js(Napi::Env env_param, const core::Any &value,
                                              const TypeInfo *type_info) {
        if (!value.has_value()) {
            return env_param.Undefined();
        }

        // Get type index from Any
        std::type_index idx = value.get_type_index();

        // Try to find registered converter (preferred method)
        auto converter = get_to_js_converter(idx);
        if (converter) {
            try {
                return converter(env_param, value);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Converter failed: " << e.what() << std::endl;
                // Fall through to fallback
            }
        }

        // FALLBACK: Direct type conversion using mangled name
        std::string mangled = value.type_name();

        // std::cerr << "[CONVERT] Type: " << mangled << ", has_value: " << value.has_value() <<
        // std::endl;

        try {
            // Double
            if (mangled == "d") {
                return Napi::Number::New(env_param, value.as<double>());
            }

            // Float
            if (mangled == "f") {
                return Napi::Number::New(env_param, static_cast<double>(value.as<float>()));
            }

            // Int (various representations)
            if (mangled == "i" || mangled == "l" || mangled == "x") {
                return Napi::Number::New(env_param, value.as<int>());
            }

            // Bool
            if (mangled == "b") {
                return Napi::Boolean::New(env_param, value.as<bool>());
            }

            // String (check for various std::string manglings)
            if (mangled.find("string") != std::string::npos ||
                mangled.find("basic_string") != std::string::npos || mangled == "Ss") {
                return Napi::String::New(env_param, value.as<std::string>());
            }

            // Vector<double> - libc++ (Clang/macOS)
            if (mangled.find("vectorIdN") != std::string::npos ||
                mangled.find("6vectorIdN") != std::string::npos) {
                try {
                    const std::vector<double> &vec = value.as<std::vector<double>>();
                    Napi::Array                arr = Napi::Array::New(env_param, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr[i] = Napi::Number::New(env_param, vec[i]);
                    }
                    return arr;
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<double>: " << e.what()
                              << std::endl;
                    return Napi::Array::New(env_param, 0);
                }
            }

            // Vector<int> - libc++ (Clang/macOS)
            if (mangled.find("vectorIiN") != std::string::npos ||
                mangled.find("6vectorIiN") != std::string::npos) {
                try {
                    const std::vector<int> &vec = value.as<std::vector<int>>();
                    Napi::Array             arr = Napi::Array::New(env_param, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr[i] = Napi::Number::New(env_param, vec[i]);
                    }
                    return arr;
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<int>: " << e.what() << std::endl;
                    return Napi::Array::New(env_param, 0);
                }
            }

            // Vector<float> - libc++ (Clang/macOS)
            if (mangled.find("vectorIfN") != std::string::npos ||
                mangled.find("6vectorIfN") != std::string::npos) {
                try {
                    const std::vector<float> &vec = value.as<std::vector<float>>();
                    Napi::Array               arr = Napi::Array::New(env_param, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr[i] = Napi::Number::New(env_param, static_cast<double>(vec[i]));
                    }
                    return arr;
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<float>: " << e.what()
                              << std::endl;
                    return Napi::Array::New(env_param, 0);
                }
            }

            // Vector<string> - libc++ (Clang/macOS)
            if ((mangled.find("vector") != std::string::npos &&
                 mangled.find("basic_string") != std::string::npos) ||
                mangled.find("6vectorINS_12basic_string") != std::string::npos) {
                try {
                    const std::vector<std::string> &vec = value.as<std::vector<std::string>>();
                    Napi::Array                     arr = Napi::Array::New(env_param, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr[i] = Napi::String::New(env_param, vec[i]);
                    }
                    return arr;
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<string>: " << e.what()
                              << std::endl;
                    return Napi::Array::New(env_param, 0);
                }
            }

            // Optional<double> - libc++ (Clang/macOS)
            if (mangled.find("optionalIdEE") != std::string::npos ||
                mangled.find("8optionalIdEE") != std::string::npos) {
                std::optional<double> opt = value.as<std::optional<double>>();
                if (opt.has_value()) {
                    return Napi::Number::New(env_param, *opt);
                }
                return env_param.Null();
            }

            // Optional<int> - libc++ (Clang/macOS)
            if (mangled.find("optionalIiEE") != std::string::npos ||
                mangled.find("8optionalIiEE") != std::string::npos) {
                std::optional<int> opt = value.as<std::optional<int>>();
                if (opt.has_value()) {
                    return Napi::Number::New(env_param, *opt);
                }
                return env_param.Null();
            }

            // Optional<float> - libc++ (Clang/macOS)
            if (mangled.find("optionalIfEE") != std::string::npos ||
                mangled.find("8optionalIfEE") != std::string::npos) {
                std::optional<float> opt = value.as<std::optional<float>>();
                if (opt.has_value()) {
                    return Napi::Number::New(env_param, static_cast<double>(*opt));
                }
                return env_param.Null();
            }

            // Optional<bool> - libc++ (Clang/macOS)
            if (mangled.find("optionalIbEE") != std::string::npos ||
                mangled.find("8optionalIbEE") != std::string::npos) {
                std::optional<bool> opt = value.as<std::optional<bool>>();
                if (opt.has_value()) {
                    return Napi::Boolean::New(env_param, *opt);
                }
                return env_param.Null();
            }

            // Optional<string> - libc++ (Clang/macOS)
            if (mangled.find("optional") != std::string::npos &&
                mangled.find("basic_string") != std::string::npos) {
                std::optional<std::string> opt = value.as<std::optional<std::string>>();
                if (opt.has_value()) {
                    return Napi::String::New(env_param, *opt);
                }
                return env_param.Null();
            }

            // Generic vector fallback (for libstdc++ / MSVC)
            if (mangled.find("vector") != std::string::npos) {
                std::cerr << "[WARN] Unknown vector type, cannot convert: " << mangled << std::endl;
                // Could try generic handling here if needed
            }

            // Generic optional fallback (for libstdc++ / MSVC)
            if (mangled.find("optional") != std::string::npos) {
                std::cerr << "[WARN] Unknown optional type, cannot convert: " << mangled
                          << std::endl;
                // Could try generic handling here if needed
            }

        } catch (const std::bad_cast &e) {
            std::cerr << "[ERROR] Type cast failed for mangled type: " << mangled << std::endl;
            return env_param.Undefined();
        } catch (const std::exception &e) {
            std::cerr << "[ERROR] Conversion failed: " << e.what() << std::endl;
            return env_param.Undefined();
        }

        // If we get here, we don't know how to convert this type
        std::cerr << "[ERROR] No converter or fallback for type: " << mangled
                  << " (type_index: " << idx.name() << ")" << std::endl;

        return env_param.Undefined();
    }

    inline core::Any JsGenerator::js_to_any(const Napi::Value &value, const TypeInfo *type_info) {
        if (value.IsUndefined() || value.IsNull()) {
            return core::Any();
        }

        // If we have type info, use it to guide conversion
        if (type_info) {
            // Try to find converter by type_index
            auto converter = get_from_js_converter(type_info->type_index);
            if (converter) {
                return converter(value);
            }

            // Fallback based on TypeInfo category
            switch (type_info->category) {
            case TypeCategory::Primitive:
                if (type_info->is_integer() && value.IsNumber()) {
                    return core::Any(value.As<Napi::Number>().Int32Value());
                }
                if (type_info->is_floating() && value.IsNumber()) {
                    return core::Any(value.As<Napi::Number>().DoubleValue());
                }
                if (type_info->name == "bool" && value.IsBoolean()) {
                    return core::Any(value.As<Napi::Boolean>().Value());
                }
                break;

            case TypeCategory::String:
                if (value.IsString()) {
                    return core::Any(value.As<Napi::String>().Utf8Value());
                }
                break;

            default:
                break;
            }
        }

        // Fallback: infer from JS type
        if (value.IsNumber()) {
            return core::Any(value.As<Napi::Number>().DoubleValue());
        }
        if (value.IsBoolean()) {
            return core::Any(value.As<Napi::Boolean>().Value());
        }
        if (value.IsString()) {
            return core::Any(value.As<Napi::String>().Utf8Value());
        }

        // Handle arrays - convert to vector<double> for numeric arrays
        if (value.IsArray()) {
            Napi::Array arr = value.As<Napi::Array>();

            if (arr.Length() == 0) {
                return core::Any(std::vector<double>());
            }

            // Check first element to determine type
            Napi::Value first_elem = arr.Get(uint32_t(0));
            
            if (first_elem.IsString()) {
                // String array
                std::vector<std::string> str_vec;
                str_vec.reserve(arr.Length());
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    Napi::Value elem = arr.Get(i);
                    if (elem.IsString()) {
                        str_vec.push_back(elem.As<Napi::String>().Utf8Value());
                    }
                }
                return core::Any(std::move(str_vec));
            } else if (first_elem.IsNumber()) {
                // Numeric array - use vector<double>
                std::vector<double> vec;
                vec.reserve(arr.Length());
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    Napi::Value elem = arr.Get(i);
                    if (elem.IsNumber()) {
                        vec.push_back(elem.As<Napi::Number>().DoubleValue());
                    }
                }
                return core::Any(std::move(vec));
            }

            // Unknown array type
            return core::Any(std::vector<double>());
        }

        return core::Any();
    }

    template <typename T> inline const TypeInfo &JsGenerator::get_type_info() const {
        return TypeRegistry::instance().get<T>();
    }

    template <typename Class>
    inline Napi::Function JsGenerator::create_wrapper_class(const std::string &class_name) {
        // Get metadata from registry
        auto &registry = core::Registry::instance();
        if (!registry.has_class<Class>()) {
            throw std::runtime_error("Class not registered: " + class_name);
        }

        // const auto &meta = registry.get<Class>();

        // Capture 'this' pointer to pass to WrappedObject
        JsGenerator *gen_ptr = this;

        // Create JavaScript constructor
        auto constructor = [gen_ptr](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();

            if (!info.IsConstructCall()) {
                Napi::TypeError::New(env, "Class constructor must be called with 'new'")
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // Create C++ instance with generator pointer
            auto *wrapped = new WrappedObject<Class>(new Class(), gen_ptr);

            // Wrap in JS object with finalizer
            Napi::Object obj = info.This().As<Napi::Object>();
            
            // Create external with finalizer to delete wrapped object
            auto external = Napi::External<WrappedObject<Class>>::New(
                env, 
                wrapped,
                [](Napi::Env env, WrappedObject<Class>* data) {
                    delete data;
                }
            );
            
            obj.Set("__cpp_instance", external);

            return obj;
        };

        Napi::Function ctor = Napi::Function::New(env, constructor, class_name);

        // Store constructor reference
        Napi::FunctionReference ctor_ref;
        ctor_ref                  = Napi::Persistent(ctor);
        constructors_[class_name] = std::move(ctor_ref);

        return ctor;
    }

    template <typename Class>
    inline void JsGenerator::bind_fields(Napi::FunctionReference          &ctor_ref,
                                         const core::ClassMetadata<Class> &meta) {
        Napi::Function ctor      = ctor_ref.Value();
        Napi::Object   prototype = ctor.Get("prototype").As<Napi::Object>();

        for (const auto &field_name : meta.fields()) {
            // Capture 'this' pointer to access converters
            JsGenerator *gen_ptr = this;

            // Getter
            auto getter = [field_name, gen_ptr](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env    env      = info.Env();
                Napi::Object this_obj = info.This().As<Napi::Object>();

                auto *wrapped = this_obj.Get("__cpp_instance")
                                    .As<Napi::External<WrappedObject<Class>>>()
                                    .Data();

                if (!wrapped || !wrapped->instance) {
                    Napi::Error::New(env, "Invalid C++ instance").ThrowAsJavaScriptException();
                    return env.Undefined();
                }

                auto       &registry = core::Registry::instance();
                const auto &meta     = registry.get<Class>();

                core::Any field_value = meta.get_field(*wrapped->instance, field_name);

                // Use the generator's any_to_js method which uses registered converters
                return gen_ptr->any_to_js(env, field_value, nullptr);
            };

            // Setter
            auto setter = [field_name, gen_ptr](const Napi::CallbackInfo &info) -> void {
                Napi::Env    env      = info.Env();
                Napi::Object this_obj = info.This().As<Napi::Object>();

                if (info.Length() < 1) {
                    Napi::TypeError::New(env, "Setter requires a value")
                        .ThrowAsJavaScriptException();
                    return;
                }

                auto *wrapped = this_obj.Get("__cpp_instance")
                                    .As<Napi::External<WrappedObject<Class>>>()
                                    .Data();

                if (!wrapped || !wrapped->instance) {
                    Napi::Error::New(env, "Invalid C++ instance").ThrowAsJavaScriptException();
                    return;
                }

                Napi::Value js_value = info[0];

                // Use the generator's js_to_any method which uses registered converters
                core::Any cpp_value = gen_ptr->js_to_any(js_value, nullptr);

                auto       &registry = core::Registry::instance();
                const auto &meta     = registry.get<Class>();

                try {
                    meta.set_field(*wrapped->instance, field_name, cpp_value);
                } catch (const std::exception &e) {
                    Napi::Error::New(env, std::string("Field setter failed: ") + e.what())
                        .ThrowAsJavaScriptException();
                }
            };

            // Define property with getter/setter
            Napi::PropertyDescriptor desc =
                Napi::PropertyDescriptor::Accessor(field_name, getter, setter);

            prototype.DefineProperty(desc);
        }
    }

    template <typename Class>
    inline void JsGenerator::bind_methods(Napi::FunctionReference          &ctor_ref,
                                          const core::ClassMetadata<Class> &meta) {
        Napi::Function ctor      = ctor_ref.Value();
        Napi::Object   prototype = ctor.Get("prototype").As<Napi::Object>();

        for (const auto &method_name : meta.methods()) {
            // Capture 'this' pointer to access converters
            JsGenerator *gen_ptr = this;

            auto method_wrapper = [method_name,
                                   gen_ptr](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env    env      = info.Env();
                Napi::Object this_obj = info.This().As<Napi::Object>();

                auto *wrapped = this_obj.Get("__cpp_instance")
                                    .As<Napi::External<WrappedObject<Class>>>()
                                    .Data();

                if (!wrapped || !wrapped->instance) {
                    Napi::Error::New(env, "Invalid C++ instance").ThrowAsJavaScriptException();
                    return env.Undefined();
                }

                // Convert arguments using js_to_any
                std::vector<core::Any> args;
                for (size_t i = 0; i < info.Length(); ++i) {
                    args.push_back(gen_ptr->js_to_any(info[i], nullptr));
                }

                // Invoke method
                auto       &registry = core::Registry::instance();
                const auto &meta     = registry.get<Class>();

                try {
                    core::Any result = meta.invoke_method(*wrapped->instance, method_name, args);

                    // Convert result to JS using any_to_js
                    return gen_ptr->any_to_js(env, result, nullptr);

                } catch (const std::exception &e) {
                    Napi::Error::New(env, std::string("Method invocation failed: ") + e.what())
                        .ThrowAsJavaScriptException();
                    return env.Undefined();
                }
            };

            prototype.Set(method_name, Napi::Function::New(env, method_wrapper, method_name));
        }
    }

    template <typename Class>
    inline JsGenerator &JsGenerator::bind_class(const std::string &custom_name) {
        auto &registry = core::Registry::instance();

        if (!registry.has_class<Class>()) {
            throw std::runtime_error("Class not registered in Rosetta: " +
                                     std::string(typeid(Class).name()));
        }

        const auto &meta       = registry.get<Class>();
        std::string class_name = custom_name.empty() ? meta.name() : custom_name;

        // Check if already bound
        if (bound_classes_.count(class_name)) {
            return *this;
        }

        // Create wrapper class
        Napi::Function          ctor     = create_wrapper_class<Class>(class_name);
        Napi::FunctionReference ctor_ref = Napi::Persistent(ctor);

        // Bind fields and methods
        bind_fields<Class>(ctor_ref, meta);
        bind_methods<Class>(ctor_ref, meta);

        // Export to module
        exports.Set(class_name, ctor);

        // Mark as bound
        bound_classes_.insert(class_name);

        return *this;
    }

    template <typename... Classes> inline JsGenerator &JsGenerator::bind_classes() {
        (bind_class<Classes>(), ...);
        return *this;
    }

    template <typename Ret, typename... Args>
    inline JsGenerator &JsGenerator::bind_function(const std::string &name, Ret (*func)(Args...)) {
        auto wrapper = [func](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();

            if (info.Length() < sizeof...(Args)) {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // This is a simplified version - would need proper arg conversion
            if constexpr (std::is_void_v<Ret>) {
                // Call void function
                return env.Undefined();
            } else {
                // Call and return result
                // Ret result = func(...);
                return env.Undefined();
            }
        };

        exports.Set(name, Napi::Function::New(env, wrapper, name));
        return *this;
    }

    inline JsGenerator &JsGenerator::add_utilities() {
        // Add version info
        auto get_version = [](const Napi::CallbackInfo &info) -> Napi::Value {
            return Napi::String::New(info.Env(), "1.0.0");
        };
        exports.Set("getVersion", Napi::Function::New(env, get_version, "getVersion"));

        // Add class list
        auto list_classes = [](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env      = info.Env();
            auto     &registry = core::Registry::instance();
            auto      classes  = registry.list_classes();

            Napi::Array result = Napi::Array::New(env, classes.size());
            for (size_t i = 0; i < classes.size(); ++i) {
                result[i] = Napi::String::New(env, classes[i]);
            }
            return result;
        };
        exports.Set("listClasses", Napi::Function::New(env, list_classes, "listClasses"));

        return *this;
    }

    // ========================================================================
    // STANDALONE HELPERS
    // ========================================================================

    template <typename T>
    inline void bind_class(JsGenerator &generator, const std::string &class_name) {
        generator.bind_class<T>(class_name);
    }

    template <typename... Classes> inline void bind_classes(JsGenerator &generator) {
        generator.bind_classes<Classes...>();
    }

} // namespace rosetta::generators::js