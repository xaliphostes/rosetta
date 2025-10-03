/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */

#include "../js_generator.h"
#include <rosetta/introspectable.h>
#include <typeinfo>

namespace rosetta {

    /**
     * @brief Register pointer type converter for any introspectable class
     * @tparam T The introspectable class type
     * @param generator The JavaScript generator to register with
     */
    template <typename T> inline void registerPointerType(JsGenerator& generator)
    {
        static_assert(
            std::is_base_of_v<Introspectable, T>, "Type must inherit from Introspectable");

        std::string typeName = std::string(typeid(T*).name());

        generator.register_type_converter(
            typeName,
            // C++ to JS: Convert T* to JavaScript T object
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                try {
                    T* ptr = std::any_cast<T*>(value);
                    if (!ptr) {
                        return env.Null();
                    }

                    // Get the constructor and create new instance
                    auto constructor = rosetta::ObjectWrapper<T>::constructor.Value();
                    auto instance = constructor.New({});

                    // Get the wrapper and copy the object
                    auto wrapper = Napi::ObjectWrap<rosetta::ObjectWrapper<T>>::Unwrap(instance);
                    *wrapper->GetCppObject() = *ptr;

                    return instance;
                } catch (const std::bad_any_cast&) {
                    return env.Null();
                } catch (...) {
                    return env.Null();
                }
            },
            // JS to C++: Convert JavaScript T object to T*
            [](const Napi::Value& js_val) -> std::any {
                if (js_val.IsNull() || js_val.IsUndefined()) {
                    return static_cast<T*>(nullptr);
                }

                try {
                    auto wrapper = Napi::ObjectWrap<rosetta::ObjectWrapper<T>>::Unwrap(
                        js_val.As<Napi::Object>());
                    return wrapper->GetCppObject();
                } catch (...) {
                    return static_cast<T*>(nullptr);
                }
            });
    }

    /**
     * @brief Register pointer converters for multiple classes
     * @tparam Classes The introspectable class types
     * @param generator The JavaScript generator to register with
     */
    template <typename... Classes> inline void registerPointerTypes(JsGenerator& generator)
    {
        (registerPointerType<Classes>(generator), ...);
    }

} // namespace rosetta