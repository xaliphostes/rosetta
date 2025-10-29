// ============================================================================
// rosetta/generators/js/type_converters.h
//
// Pre-built type converters for common C++ types using TypeInfo system
// ============================================================================
#pragma once

#include "js_generator.h"
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rosetta::generators::js {

    /**
     * @brief Register converters for std::vector<T>
     */
    template <typename T> inline void register_vector_converter(JsGenerator &gen) {
        // Optional: keep your registry entry
        TypeRegistry::instance().register_type<std::vector<T>>();

        // C++ vector<T> -> JS Array
        gen.register_converter<std::vector<T>>(
            [](Napi::Env env, const rosetta::core::Any &val) -> Napi::Value {
                // std::cerr << "[TYPE CONVERTER]: 1-" << std::endl;
                const auto &vec = val.as<std::vector<T>>();
                // std::cerr << "[TYPE CONVERTER]: 2-" << std::endl;
                Napi::Array arr = Napi::Array::New(env, vec.size());
                // std::cerr << "[TYPE CONVERTER]: " << vec.size() << std::endl;

                for (size_t i = 0; i < vec.size(); ++i) {
                    if constexpr (std::is_integral_v<T>) {
                        arr[i] = Napi::Number::New(env, static_cast<double>(vec[i]));
                    } else if constexpr (std::is_floating_point_v<T>) {
                        arr[i] = Napi::Number::New(env, static_cast<double>(vec[i]));
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        arr[i] = Napi::String::New(env, vec[i]);
                    } else if constexpr (std::is_same_v<T, bool>) {
                        arr[i] = Napi::Boolean::New(env, vec[i]);
                    } else {
                        // Unsupported T: fall back to undefined (or throw)
                        arr[i] = env.Undefined();
                    }
                }
                return arr;
            },
            // JS Array -> C++ vector<T>
            [](const Napi::Value &val) -> rosetta::core::Any {
                if (!val.IsArray()) {
                    return rosetta::core::Any();
                }

                // std::cerr << "[TYPE CONVERTER]: 1-" << std::endl;

                Napi::Array    arr = val.As<Napi::Array>();
                std::vector<T> vec;
                vec.reserve(arr.Length());

                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    Napi::Value elem = arr[i];

                    if constexpr (std::is_integral_v<T>) {
                        vec.push_back(static_cast<T>(elem.As<Napi::Number>().Int64Value()));
                    } else if constexpr (std::is_floating_point_v<T>) {
                        vec.push_back(static_cast<T>(elem.As<Napi::Number>().DoubleValue()));
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        vec.push_back(elem.As<Napi::String>().Utf8Value());
                    } else if constexpr (std::is_same_v<T, bool>) {
                        vec.push_back(elem.As<Napi::Boolean>().Value());
                    } else {
                        // Unsupported T: skip or push a default-constructed T
                        // vec.emplace_back();
                    }
                }

                return rosetta::core::Any(vec);
            });
    }

    /**
     * @brief Register converters for std::array<T, N>
     */
    template <typename T, size_t N> inline void register_array_converter(JsGenerator &gen) {
        // std::string type_name = typeid(std::array<T, N>).name();

        gen.register_converter<std::array<T, N>>(
            // type_name,
            [](Napi::Env env, const core::Any &val) -> Napi::Value {
                const auto &arr_cpp = val.as<std::array<T, N>>();
                Napi::Array arr_js  = Napi::Array::New(env, N);

                for (size_t i = 0; i < N; ++i) {
                    if constexpr (std::is_arithmetic_v<T>) {
                        arr_js[i] = Napi::Number::New(env, static_cast<double>(arr_cpp[i]));
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        arr_js[i] = Napi::String::New(env, arr_cpp[i]);
                    }
                }

                return arr_js;
            },
            [](const Napi::Value &val) -> core::Any {
                if (!val.IsArray()) {
                    return core::Any();
                }

                Napi::Array      arr_js = val.As<Napi::Array>();
                std::array<T, N> arr_cpp;

                size_t count = std::min(static_cast<size_t>(arr_js.Length()), N);
                for (size_t i = 0; i < count; ++i) {
                    Napi::Value elem = arr_js[i];
                    if constexpr (std::is_arithmetic_v<T>) {
                        arr_cpp[i] = static_cast<T>(elem.As<Napi::Number>().DoubleValue());
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        arr_cpp[i] = elem.As<Napi::String>().Utf8Value();
                    }
                }

                return core::Any(arr_cpp);
            });
    }

    /**
     * @brief Register converters for std::map<K, V>
     */
    template <typename K, typename V> inline void register_map_converter(JsGenerator &gen) {
        std::string type_name = typeid(std::map<K, V>).name();

        gen.register_converter<std::map<K, V>>(
            // type_name,
            [](Napi::Env env, const core::Any &val) -> Napi::Value {
                const auto  &map = val.as<std::map<K, V>>();
                Napi::Object obj = Napi::Object::New(env);

                for (const auto &[key, value] : map) {
                    std::string key_str;
                    if constexpr (std::is_same_v<K, std::string>) {
                        key_str = key;
                    } else {
                        key_str = std::to_string(key);
                    }

                    if constexpr (std::is_arithmetic_v<V>) {
                        obj.Set(key_str, Napi::Number::New(env, static_cast<double>(value)));
                    } else if constexpr (std::is_same_v<V, std::string>) {
                        obj.Set(key_str, Napi::String::New(env, value));
                    }
                }

                return obj;
            },
            [](const Napi::Value &val) -> core::Any {
                if (!val.IsObject()) {
                    return core::Any();
                }

                Napi::Object   obj = val.As<Napi::Object>();
                std::map<K, V> map;

                Napi::Array keys = obj.GetPropertyNames();
                for (uint32_t i = 0; i < keys.Length(); ++i) {
                    Napi::Value key_val = keys[i];
                    std::string key_str = key_val.As<Napi::String>().Utf8Value();

                    K key;
                    if constexpr (std::is_same_v<K, std::string>) {
                        key = key_str;
                    } else {
                        key = static_cast<K>(std::stod(key_str));
                    }

                    Napi::Value value_val = obj.Get(key_str);
                    if constexpr (std::is_arithmetic_v<V>) {
                        map[key] = static_cast<V>(value_val.As<Napi::Number>().DoubleValue());
                    } else if constexpr (std::is_same_v<V, std::string>) {
                        map[key] = value_val.As<Napi::String>().Utf8Value();
                    }
                }

                return core::Any(map);
            });
    }

    /**
     * @brief Register converters for std::optional<T>
     */
    template <typename T> inline void register_optional_converter(JsGenerator &gen) {
        // std::string type_name = typeid(std::optional<T>).name();

        gen.register_converter<std::optional<T>>(
            // type_name,
            [](Napi::Env env, const core::Any &val) -> Napi::Value {
                const auto &opt = val.as<std::optional<T>>();

                if (!opt.has_value()) {
                    return env.Null();
                }

                if constexpr (std::is_arithmetic_v<T>) {
                    return Napi::Number::New(env, static_cast<double>(*opt));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return Napi::String::New(env, *opt);
                } else if constexpr (std::is_same_v<T, bool>) {
                    return Napi::Boolean::New(env, *opt);
                }

                return env.Undefined();
            },
            [](const Napi::Value &val) -> core::Any {
                if (val.IsNull() || val.IsUndefined()) {
                    return core::Any(std::optional<T>());
                }

                if constexpr (std::is_arithmetic_v<T>) {
                    return core::Any(
                        std::optional<T>(static_cast<T>(val.As<Napi::Number>().DoubleValue())));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return core::Any(std::optional<T>(val.As<Napi::String>().Utf8Value()));
                } else if constexpr (std::is_same_v<T, bool>) {
                    return core::Any(std::optional<T>(val.As<Napi::Boolean>().Value()));
                }

                return core::Any(std::optional<T>());
            });
    }

    /**
     * @brief Register converters for std::shared_ptr<T>
     */
    template <typename T> inline void register_shared_ptr_converter(JsGenerator &gen) {
        std::string type_name = typeid(std::shared_ptr<T>).name();

        gen.register_converter(
            type_name,
            [](Napi::Env env, const core::Any &val) -> Napi::Value {
                const auto &ptr = val.as<std::shared_ptr<T>>();

                if (!ptr) {
                    return env.Null();
                }

                // Would need to wrap the object properly
                // This is a simplified version
                return env.Undefined();
            },
            [](const Napi::Value &val) -> core::Any {
                // Complex conversion - would need proper handling
                return core::Any(std::shared_ptr<T>());
            });
    }

    /**
     * @brief Convenience function to register all common converters
     */
    inline void register_common_converters(JsGenerator &gen) {
        // Basic vectors
        register_vector_converter<int>(gen);
        register_vector_converter<double>(gen);
        register_vector_converter<float>(gen);
        register_vector_converter<std::string>(gen);

        // Common arrays
        register_array_converter<double, 3>(gen);
        register_array_converter<float, 3>(gen);
        register_array_converter<int, 3>(gen);

        // Maps
        register_map_converter<std::string, int>(gen);
        register_map_converter<std::string, double>(gen);
        register_map_converter<std::string, std::string>(gen);

        // Optionals
        register_optional_converter<int>(gen);
        register_optional_converter<double>(gen);
        register_optional_converter<std::string>(gen);
        register_optional_converter<bool>(gen);
    }

} // namespace rosetta::generators::js