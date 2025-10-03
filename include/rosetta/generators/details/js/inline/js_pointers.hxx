/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 * Proper pointer handling with non-owning shared_ptr wrapper
 */

#include "../js_generator.h"
#include <rosetta/introspectable.h>
#include <rosetta/types.h>
#include <typeinfo>
#include <memory>
#include <unordered_map>

namespace rosetta {

    /**
     * @brief Registry for pointer wrapper constructors
     */
    class PointerWrapperRegistry {
    public:
        static PointerWrapperRegistry& instance() {
            static PointerWrapperRegistry registry;
            return registry;
        }

        template <typename T>
        void ensure_initialized(Napi::Env env, Napi::Object exports) {
            std::string type_name = typeid(T).name();
            if (initialized_.find(type_name) != initialized_.end()) {
                return; // Already initialized
            }

            // Initialize ObjectWrapper<T> if not already done
            std::string class_name = T::getStaticTypeInfo().class_name + "_Ptr_Internal";
            ObjectWrapper<T>::Init(env, exports, class_name);
            initialized_.insert(type_name);
        }

        template <typename T>
        bool is_initialized() const {
            return initialized_.find(typeid(T).name()) != initialized_.end();
        }

    private:
        std::unordered_set<std::string> initialized_;
        PointerWrapperRegistry() = default;
    };

    /**
     * @brief Create a JavaScript wrapper for a C++ pointer (non-owning)
     */
    template <typename T>
    inline Napi::Value wrapPointer(Napi::Env env, T* ptr) {
        if (!ptr) {
            return env.Null();
        }

        try {
            auto constructor = ObjectWrapper<T>::constructor.Value();
            auto instance = constructor.New({});
            auto wrapper = Napi::ObjectWrap<ObjectWrapper<T>>::Unwrap(instance);
            
            wrapper->SetNonOwningPointer(ptr);
            
            return instance;
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Failed to wrap pointer: ") + e.what())
                .ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    /**
     * @brief Extract C++ pointer from JavaScript wrapper
     */
    template <typename T>
    inline T* unwrapPointer(const Napi::Value& js_val) {
        if (js_val.IsNull() || js_val.IsUndefined()) {
            return nullptr;
        }

        try {
            if (!js_val.IsObject()) {
                return nullptr;
            }

            auto wrapper = Napi::ObjectWrap<ObjectWrapper<T>>::Unwrap(
                js_val.As<Napi::Object>());
            
            return wrapper->GetCppObject();
        } catch (...) {
            return nullptr;
        }
    }

    /**
     * @brief Register pointer type converter for any introspectable class
     * 
     * This converter handles raw pointers by wrapping them in a JavaScript object
     * that references the pointed-to C++ object. Changes to the JavaScript object
     * affect the original C++ object.
     * 
     * @tparam T The introspectable class type
     * @param generator The JavaScript generator to register with
     */
    template <typename T> 
    inline void registerPointerType(JsGenerator& generator)
    {
        static_assert(
            std::is_base_of_v<Introspectable, T>, 
            "Type must inherit from Introspectable");

        // Ensure ObjectWrapper<T> is initialized
        PointerWrapperRegistry::instance().ensure_initialized<T>(
            generator.env, generator.exports);

        // Use the same type name that getTypeName<T*>() returns
        std::string typeName = rosetta::getTypeName<T*>();

        // Register bidirectional converter
        generator.register_type_converter(
            typeName,
            // C++ pointer → JavaScript object
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                try {
                    T* ptr = std::any_cast<T*>(value);
                    return wrapPointer(env, ptr);
                } catch (const std::bad_any_cast& e) {
                    Napi::Error::New(env, 
                        std::string("Bad pointer cast: ") + e.what())
                        .ThrowAsJavaScriptException();
                    return env.Null();
                }
            },
            // JavaScript object → C++ pointer
            [](const Napi::Value& js_val) -> std::any {
                return unwrapPointer<T>(js_val);
            }
        );
    }

    /**
     * @brief Register pointer converters for multiple classes
     */
    template <typename... Classes> 
    inline void registerPointerTypes(JsGenerator& generator)
    {
        (registerPointerType<Classes>(generator), ...);
    }

} // namespace rosetta