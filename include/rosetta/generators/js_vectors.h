/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#pragma once
#include <rosetta/generators/js.h>
#include <rosetta/type_registry.h>
#include <type_traits>
#include <typeinfo>

namespace rosetta {

    // ============================================================================
    // Type conversion helpers
    // ============================================================================

    template <typename T> inline Napi::Value toNapiValue(Napi::Env env, const T& value)
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return Napi::String::New(env, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return Napi::Boolean::New(env, value);
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) > 4 || std::is_same_v<T, size_t>) {
                return Napi::Number::New(env, static_cast<double>(value));
            } else if constexpr (std::is_signed_v<T>) {
                return Napi::Number::New(env, static_cast<int32_t>(value));
            } else {
                return Napi::Number::New(env, static_cast<uint32_t>(value));
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            return Napi::Number::New(env, static_cast<double>(value));
        } else {
            return env.Undefined();
        }
    }

    template <typename T> inline T fromNapiValue(const Napi::Value& value)
    {
        if constexpr (std::is_same_v<T, std::string>) {
            if (!value.IsString()) {
                throw Napi::TypeError::New(value.Env(), "Expected string");
            }
            return value.As<Napi::String>().Utf8Value();
        } else if constexpr (std::is_same_v<T, bool>) {
            if (!value.IsBoolean()) {
                throw Napi::TypeError::New(value.Env(), "Expected boolean");
            }
            return value.As<Napi::Boolean>().Value();
        } else if constexpr (std::is_integral_v<T>) {
            if (!value.IsNumber()) {
                throw Napi::TypeError::New(value.Env(), "Expected number");
            }
            if constexpr (sizeof(T) > 4 || std::is_same_v<T, size_t>) {
                if constexpr (std::is_signed_v<T>) {
                    return static_cast<T>(value.As<Napi::Number>().Int64Value());
                } else {
                    return static_cast<T>(value.As<Napi::Number>().Int64Value());
                }
            } else if constexpr (std::is_signed_v<T>) {
                return static_cast<T>(value.As<Napi::Number>().Int32Value());
            } else {
                return static_cast<T>(value.As<Napi::Number>().Uint32Value());
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if (!value.IsNumber()) {
                throw Napi::TypeError::New(value.Env(), "Expected number");
            }
            if constexpr (std::is_same_v<T, float>) {
                return static_cast<float>(value.As<Napi::Number>().FloatValue());
            } else {
                return value.As<Napi::Number>().DoubleValue();
            }
        } else {
            return T {};
        }
    }

    // ============================================================================
    // Auto vector registration
    // ============================================================================

    /**
     * @brief Register vector type converter using automatic type name from typeid
     * @tparam T Element type of the vector
     * @param generator The JavaScript generator to register with
     */
    template <typename T> inline void registerVectorType(JsGenerator& generator)
    {
        // Use the mangled name from typeid as the key
        std::string typeName = typeid(std::vector<T>).name();

        generator.register_type_converter(
            typeName,
            // C++ -> JS
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                try {
                    const auto& vec = std::any_cast<const std::vector<T>&>(value);
                    auto arr = Napi::Array::New(env, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr.Set(i, toNapiValue<T>(env, vec[i]));
                    }
                    return arr;
                } catch (const std::bad_any_cast&) {
                    auto vec = std::any_cast<std::vector<T>>(value);
                    auto arr = Napi::Array::New(env, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr.Set(i, toNapiValue<T>(env, vec[i]));
                    }
                    return arr;
                }
            },
            // JS -> C++
            [](const Napi::Value& js_val) -> std::any {
                if (!js_val.IsArray()) {
                    throw Napi::TypeError::New(js_val.Env(), "Expected array");
                }

                auto arr = js_val.As<Napi::Array>();
                std::vector<T> vec;
                vec.reserve(arr.Length());

                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    vec.push_back(fromNapiValue<T>(arr.Get(i)));
                }

                return vec;
            });
    }

    /**
     * @brief Register type alias converter (for aliases like "using Vertices =
     * std::vector<double>")
     * @tparam AliasType The type alias
     * @tparam ElementType The element type of the underlying vector
     * @param generator The JavaScript generator to register with
     */
    template <typename AliasType, typename ElementType>
    inline void registerTypeAlias(JsGenerator& generator)
    {
        std::string aliasName = typeid(AliasType).name();

        generator.register_type_converter(
            aliasName,
            // C++ -> JS
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                try {
                    const auto& vec = std::any_cast<const AliasType&>(value);
                    auto arr = Napi::Array::New(env, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr.Set(i, toNapiValue<ElementType>(env, vec[i]));
                    }
                    return arr;
                } catch (const std::bad_any_cast&) {
                    auto vec = std::any_cast<AliasType>(value);
                    auto arr = Napi::Array::New(env, vec.size());
                    for (size_t i = 0; i < vec.size(); ++i) {
                        arr.Set(i, toNapiValue<ElementType>(env, vec[i]));
                    }
                    return arr;
                }
            },
            // JS -> C++
            [](const Napi::Value& js_val) -> std::any {
                if (!js_val.IsArray()) {
                    throw Napi::TypeError::New(js_val.Env(), "Expected array");
                }

                auto arr = js_val.As<Napi::Array>();
                AliasType vec;
                vec.reserve(arr.Length());

                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    vec.push_back(fromNapiValue<ElementType>(arr.Get(i)));
                }

                return vec;
            });
    }

    /**
     * @brief Register all common vector types
     * @param generator The JavaScript generator to register with
     */
    inline void registerCommonVectorTypes(JsGenerator& generator)
    {
        // Integer types
        registerVectorType<int>(generator);
        registerVectorType<unsigned int>(generator);
        registerVectorType<int32_t>(generator);
        registerVectorType<uint32_t>(generator);
        registerVectorType<int64_t>(generator);
        registerVectorType<uint64_t>(generator);
        registerVectorType<size_t>(generator);

        // Floating point types
        registerVectorType<float>(generator);
        registerVectorType<double>(generator);

        // Other basic types
        registerVectorType<bool>(generator);
        registerVectorType<char>(generator);
        registerVectorType<std::string>(generator);
    }

} // namespace rosetta