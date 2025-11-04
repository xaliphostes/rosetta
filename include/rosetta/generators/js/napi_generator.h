// ============================================================================
// JavaScript/Node.js binding generator using Rosetta introspection
// No inheritance or wrapping required - pure non-intrusive approach
// ============================================================================
#pragma once

#include <rosetta/rosetta.h>
#include <napi.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::bindings {

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
    template <typename T>
    class JsClassWrapper : public Napi::ObjectWrap<JsClassWrapper<T>> {
    public:
        static Napi::FunctionReference constructor;

        /**
         * @brief Initialize and export the class to JavaScript
         */
        static Napi::Object Init(Napi::Env env, Napi::Object exports, 
                                const std::string &js_name) {
            // Get metadata from registry
            const auto &meta = core::Registry::instance().get<T>();

            // Build property descriptors
            std::vector<typename Napi::ObjectWrap<JsClassWrapper<T>>::PropertyDescriptor> properties;

            // Add fields as properties
            for (const auto &field_name : meta.fields()) {
                // Capture field_name by value in the lambda
                auto getter = [field_name](const Napi::CallbackInfo &info) -> Napi::Value {
                    auto* wrapper = Napi::ObjectWrap<JsClassWrapper<T>>::Unwrap(info.This().As<Napi::Object>());
                    return wrapper->GetField(info.Env(), field_name);
                };
                
                auto setter = [field_name](const Napi::CallbackInfo &info, const Napi::Value &value) {
                    auto* wrapper = Napi::ObjectWrap<JsClassWrapper<T>>::Unwrap(info.This().As<Napi::Object>());
                    wrapper->SetField(info.Env(), field_name, value);
                };
                
                properties.push_back(
                    Napi::ObjectWrap<JsClassWrapper<T>>::InstanceAccessor(
                        field_name.c_str(),
                        getter,
                        setter
                    )
                );
            }

            // Add methods
            for (const auto &method_name : meta.methods()) {
                // Capture method_name by value in the lambda
                auto method = [method_name](const Napi::CallbackInfo &info) -> Napi::Value {
                    auto* wrapper = Napi::ObjectWrap<JsClassWrapper<T>>::Unwrap(info.This().As<Napi::Object>());
                    return wrapper->CallMethod(info.Env(), method_name, info);
                };
                
                properties.push_back(
                    Napi::ObjectWrap<JsClassWrapper<T>>::InstanceMethod(
                        method_name.c_str(),
                        method
                    )
                );
            }

            // Define the class
            Napi::Function func = Napi::ObjectWrap<JsClassWrapper<T>>::DefineClass(
                env, js_name.c_str(), properties
            );

            constructor = Napi::Persistent(func);
            constructor.SuppressDestruct();

            exports.Set(js_name, func);
            return exports;
        }

        /**
         * @brief Constructor from JavaScript
         */
        JsClassWrapper(const Napi::CallbackInfo &info) 
            : Napi::ObjectWrap<JsClassWrapper<T>>(info) {
            
            Napi::Env env = info.Env();
            const auto &meta = core::Registry::instance().get<T>();
            const auto &ctors = meta.constructors();

            // Find matching constructor by argument count
            for (const auto &ctor : ctors) {
                // Try to call with matching argument count
                // For simplicity, we'll just try default constructor first
                if (info.Length() == 0) {
                    try {
                        std::vector<core::Any> args;
                        core::Any result = ctor(args);
                        cpp_object_ = result.as<T>();
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
                Napi::Error::New(env, "No matching constructor found")
                    .ThrowAsJavaScriptException();
            }
        }

        /**
         * @brief Get the underlying C++ object
         */
        T &GetCppObject() { return cpp_object_; }
        const T &GetCppObject() const { return cpp_object_; }

    private:
        T cpp_object_;

        /**
         * @brief Property getter callback
         */
        Napi::Value GetField(Napi::Env env, const std::string &field_name) {
            try {
                const auto &meta = core::Registry::instance().get<T>();
                core::Any value = meta.get_field(cpp_object_, field_name);
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
                const auto &meta = core::Registry::instance().get<T>();
                std::type_index field_type = meta.get_field_type(field_name);
                core::Any cpp_value = js_to_any(value, field_type);
                meta.set_field(cpp_object_, field_name, cpp_value);
            } catch (const std::exception &e) {
                Napi::Error::New(env, std::string("Failed to set field: ") + e.what())
                    .ThrowAsJavaScriptException();
            }
        }

        /**
         * @brief Method call callback
         */
        Napi::Value CallMethod(Napi::Env env, const std::string &method_name, const Napi::CallbackInfo &info) {
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
    template <typename T>
    Napi::FunctionReference JsClassWrapper<T>::constructor;

    // ============================================================================
    // JavaScript Binding Generator
    // ============================================================================

    /**
     * @brief Main generator class for JavaScript bindings
     */
    class JsBindingGenerator {
    public:
        JsBindingGenerator(Napi::Env env, Napi::Object exports)
            : env_(env), exports_(exports) {}

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param js_name Optional JavaScript name (uses C++ name if empty)
         */
        template <typename T>
        JsBindingGenerator &bind_class(const std::string &js_name = "") {
            // Get metadata from registry
            const auto &meta = core::Registry::instance().get<T>();
            std::string final_name = js_name.empty() ? meta.name() : js_name;

            // Initialize the wrapper
            JsClassWrapper<T>::Init(env_, exports_, final_name);

            return *this;
        }

        /**
         * @brief Add utility functions
         */
        JsBindingGenerator &add_utilities() {
            // List all registered classes
            exports_.Set("listClasses",
                Napi::Function::New(env_, [](const Napi::CallbackInfo &info) {
                    auto env = info.Env();
                    const auto &classes = core::Registry::instance().list_classes();
                    auto arr = Napi::Array::New(env, classes.size());
                    for (size_t i = 0; i < classes.size(); ++i) {
                        arr.Set(i, Napi::String::New(env, classes[i]));
                    }
                    return arr;
                }));

            return *this;
        }

    private:
        Napi::Env env_;
        Napi::Object exports_;
    };

    // ============================================================================
    // Helper Macros
    // ============================================================================

    /**
     * @brief Bind a class to JavaScript (simplified macro)
     */
    #define ROSETTA_BIND_JS_CLASS(Generator, ClassName) \
        Generator.bind_class<ClassName>(#ClassName)

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes>
    void bind_classes(JsBindingGenerator &gen) {
        (gen.bind_class<Classes>(), ...);
    }

} // namespace rosetta::bindings