// ============================================================================
// rosetta/generators/javascript/napi_type_converter.h
//
// Système de conversion de types entre C++ et JavaScript (node-addon-api)
// ============================================================================
#pragma once

#include "../../traits/container_traits.h"
#include "../../traits/pointer_traits.h"
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <napi.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace rosetta::generators::js {

    /**
     * @brief Exception pour les erreurs de conversion
     */
    class NapiConversionError : public std::runtime_error {
    public:
        explicit NapiConversionError(const std::string &msg) : std::runtime_error(msg) {}
    };

    /**
     * @brief Interface de base pour les convertisseurs de types
     */
    class INapiTypeConverter {
    public:
        virtual ~INapiTypeConverter() = default;

        /**
         * @brief Convertit une valeur C++ en valeur JavaScript
         */
        virtual Napi::Value to_napi(Napi::Env env, const void *cpp_value) const = 0;

        /**
         * @brief Convertit une valeur JavaScript en valeur C++
         */
        virtual void from_napi(Napi::Env env, Napi::Value js_value, void *cpp_value) const = 0;

        /**
         * @brief Obtient le nom du type
         */
        virtual std::string type_name() const = 0;
    };

    /**
     * @brief Convertisseur typé
     * @tparam T Type C++
     */
    template <typename T> class NapiTypeConverter : public INapiTypeConverter {
    public:
        Napi::Value to_napi(Napi::Env env, const void *cpp_value) const override {
            return to_napi_impl(env, *static_cast<const T *>(cpp_value));
        }

        void from_napi(Napi::Env env, Napi::Value js_value, void *cpp_value) const override {
            *static_cast<T *>(cpp_value) = from_napi_impl(env, js_value);
        }

        std::string type_name() const override { return typeid(T).name(); }

        virtual Napi::Value to_napi_impl(Napi::Env env, const T &value) const         = 0;
        virtual T           from_napi_impl(Napi::Env env, Napi::Value js_value) const = 0;
    };

    // ============================================================================
    // REGISTRY DE CONVERTISSEURS
    // ============================================================================

    /**
     * @brief Registry singleton pour les convertisseurs de types
     */
    class TypeConverterRegistry {
        std::unordered_map<std::type_index, std::unique_ptr<INapiTypeConverter>> converters_;
        std::unordered_map<std::string, std::type_index>                         class_wrappers_;

        TypeConverterRegistry();
        TypeConverterRegistry(const TypeConverterRegistry &)            = delete;
        TypeConverterRegistry &operator=(const TypeConverterRegistry &) = delete;

    public:
        /**
         * @brief Obtient l'instance singleton
         */
        static TypeConverterRegistry &instance();

        /**
         * @brief Enregistre un convertisseur pour un type
         */
        template <typename T>
        void register_converter(std::unique_ptr<NapiTypeConverter<T>> converter);

        /**
         * @brief Enregistre un wrapper de classe
         */
        template <typename T> void register_class_wrapper(const std::string &class_name);

        /**
         * @brief Obtient le convertisseur pour un type
         */
        template <typename T> const NapiTypeConverter<T> *get_converter() const;

        /**
         * @brief Convertit une valeur C++ en valeur JavaScript
         */
        template <typename T> Napi::Value to_napi(Napi::Env env, const T &value) const;

        /**
         * @brief Convertit une valeur JavaScript en valeur C++
         */
        template <typename T> T from_napi(Napi::Env env, Napi::Value js_value) const;

        /**
         * @brief Vérifie si un convertisseur est enregistré
         */
        template <typename T> bool has_converter() const;

        /**
         * @brief Enregistre automatiquement un conteneur si possible
         */
        template <typename Container> void register_container_if_needed();

        // Generic function to convert JS value → core::Any
        core::Any to_any(Napi::Env env, Napi::Value js) const;

    private:
        /**
         * @brief Enregistre les types prédéfinis
         */
        void register_builtin_types();
    };

    // ============================================================================
    // CONVERTISSEURS POUR TYPES PRIMITIFS
    // ============================================================================

    /**
     * @brief Convertisseur pour bool
     */
    class BoolConverter : public NapiTypeConverter<bool> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const bool &value) const override {
            return Napi::Boolean::New(env, value);
        }

        bool from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsBoolean()) {
                throw NapiConversionError("Expected a boolean");
            }
            return js_value.As<Napi::Boolean>().Value();
        }
    };

    /**
     * @brief Convertisseur pour double
     */
    class DoubleConverter : public NapiTypeConverter<double> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const double &value) const override {
            return Napi::Number::New(env, value);
        }

        double from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsNumber()) {
                throw NapiConversionError("Expected a number");
            }
            return js_value.As<Napi::Number>().DoubleValue();
        }
    };

    /**
     * @brief Convertisseur pour float
     */
    class FloatConverter : public NapiTypeConverter<float> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const float &value) const override {
            return Napi::Number::New(env, static_cast<double>(value));
        }

        float from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsNumber()) {
                throw NapiConversionError("Expected a number");
            }
            return static_cast<float>(js_value.As<Napi::Number>().DoubleValue());
        }
    };

    /**
     * @brief Convertisseur pour int32
     */
    class Int32Converter : public NapiTypeConverter<int32_t> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const int32_t &value) const override {
            return Napi::Number::New(env, value);
        }

        int32_t from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsNumber()) {
                throw NapiConversionError("Expected a number");
            }
            return js_value.As<Napi::Number>().Int32Value();
        }
    };

    /**
     * @brief Convertisseur pour uint32
     */
    class UInt32Converter : public NapiTypeConverter<uint32_t> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const uint32_t &value) const override {
            return Napi::Number::New(env, value);
        }

        uint32_t from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsNumber()) {
                throw NapiConversionError("Expected a number");
            }
            return js_value.As<Napi::Number>().Uint32Value();
        }
    };

    /**
     * @brief Convertisseur pour int64
     */
    class Int64Converter : public NapiTypeConverter<int64_t> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const int64_t &value) const override {
            return Napi::Number::New(env, static_cast<double>(value));
        }

        int64_t from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsNumber()) {
                throw NapiConversionError("Expected a number");
            }
            return js_value.As<Napi::Number>().Int64Value();
        }
    };

    /**
     * @brief Convertisseur pour std::string
     */
    class StringConverter : public NapiTypeConverter<std::string> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const std::string &value) const override {
            return Napi::String::New(env, value);
        }

        std::string from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsString()) {
                throw NapiConversionError("Expected a string");
            }
            return js_value.As<Napi::String>().Utf8Value();
        }
    };

    // ============================================================================
    // CONVERTISSEURS POUR CONTENEURS STL
    // ============================================================================

    /**
     * @brief Convertisseur pour std::vector<T>
     */
    template <typename T> class VectorConverter : public NapiTypeConverter<std::vector<T>> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const std::vector<T> &value) const override {
            auto &registry = TypeConverterRegistry::instance();

            Napi::Array arr = Napi::Array::New(env, value.size());

            for (size_t i = 0; i < value.size(); ++i) {
                arr[i] = registry.to_napi(env, value[i]);
            }

            return arr;
        }

        std::vector<T> from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsArray()) {
                throw NapiConversionError("Expected an array");
            }

            auto       &registry = TypeConverterRegistry::instance();
            Napi::Array arr      = js_value.As<Napi::Array>();

            std::vector<T> result;
            result.reserve(arr.Length());

            for (uint32_t i = 0; i < arr.Length(); ++i) {
                result.push_back(registry.from_napi<T>(env, arr[i]));
            }

            return result;
        }
    };

    /**
     * @brief Convertisseur pour std::array<T, N>
     */
    template <typename T, size_t N>
    class ArrayConverter : public NapiTypeConverter<std::array<T, N>> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const std::array<T, N> &value) const override {
            auto &registry = TypeConverterRegistry::instance();

            Napi::Array arr = Napi::Array::New(env, N);

            for (size_t i = 0; i < N; ++i) {
                arr[i] = registry.to_napi(env, value[i]);
            }

            return arr;
        }

        std::array<T, N> from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsArray()) {
                throw NapiConversionError("Expected an array");
            }

            auto       &registry = TypeConverterRegistry::instance();
            Napi::Array arr      = js_value.As<Napi::Array>();

            if (arr.Length() != N) {
                throw NapiConversionError("Array length mismatch");
            }

            std::array<T, N> result;

            for (size_t i = 0; i < N; ++i) {
                result[i] = registry.from_napi<T>(env, arr[i]);
            }

            return result;
        }
    };

    /**
     * @brief Convertisseur pour std::map<K, V>
     */
    template <typename K, typename V>
    class MapConverter : public NapiTypeConverter<std::map<K, V>> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const std::map<K, V> &value) const override {
            auto &registry = TypeConverterRegistry::instance();

            Napi::Object obj = Napi::Object::New(env);

            for (const auto &[key, val] : value) {
                std::string key_str;
                if constexpr (std::is_same_v<K, std::string>) {
                    key_str = key;
                } else {
                    key_str = std::to_string(key);
                }

                obj.Set(key_str, registry.to_napi(env, val));
            }

            return obj;
        }

        std::map<K, V> from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (!js_value.IsObject()) {
                throw NapiConversionError("Expected an object");
            }

            auto        &registry = TypeConverterRegistry::instance();
            Napi::Object obj      = js_value.As<Napi::Object>();

            std::map<K, V> result;

            Napi::Array property_names = obj.GetPropertyNames();

            for (uint32_t i = 0; i < property_names.Length(); ++i) {
                Napi::Value key_value = property_names[i];
                std::string key_str   = key_value.As<Napi::String>().Utf8Value();

                K key;
                if constexpr (std::is_same_v<K, std::string>) {
                    key = key_str;
                } else if constexpr (std::is_integral_v<K>) {
                    key = static_cast<K>(std::stoll(key_str));
                } else if constexpr (std::is_floating_point_v<K>) {
                    key = static_cast<K>(std::stod(key_str));
                }

                V val       = registry.from_napi<V>(env, obj.Get(key_value));
                result[key] = val;
            }

            return result;
        }
    };

    /**
     * @brief Convertisseur pour std::optional<T>
     */
    template <typename T> class OptionalConverter : public NapiTypeConverter<std::optional<T>> {
    public:
        Napi::Value to_napi_impl(Napi::Env env, const std::optional<T> &value) const override {
            if (!value.has_value()) {
                return env.Undefined();
            }

            auto &registry = TypeConverterRegistry::instance();
            return registry.to_napi(env, *value);
        }

        std::optional<T> from_napi_impl(Napi::Env env, Napi::Value js_value) const override {
            if (js_value.IsUndefined() || js_value.IsNull()) {
                return std::nullopt;
            }

            auto &registry = TypeConverterRegistry::instance();
            return registry.from_napi<T>(env, js_value);
        }
    };

    // ============================================================================
    // CONVERTISSEUR POUR FONCTIONS (FUNCTORS)
    // ============================================================================

    /**
     * @brief Convertisseur pour std::function
     * Supporte les conversions bidirectionnelles C++ ↔ JS
     */
    template <typename Ret, typename... Args>
    class FunctionConverter : public NapiTypeConverter<std::function<Ret(Args...)>> {
        // Structure pour stocker une référence persistante à une fonction JS
        struct JsFunctionHolder {
            Napi::FunctionReference func_ref;
            Napi::Env               env;

            JsFunctionHolder(Napi::Env e, Napi::Function f) : env(e) {
                func_ref = Napi::Persistent(f);
            }
        };

    public:
        Napi::Value to_napi_impl(Napi::Env                          env,
                                 const std::function<Ret(Args...)> &value) const override {
            // Créer une fonction JavaScript qui wrapp la fonction C++
            auto callback = [value](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env env = info.Env();

                if (info.Length() != sizeof...(Args)) {
                    Napi::TypeError::New(env, "Wrong number of arguments")
                        .ThrowAsJavaScriptException();
                    return env.Undefined();
                }

                try {
                    auto &registry = TypeConverterRegistry::instance();

                    // Convertir les arguments JS → C++
                    auto args_tuple =
                        convert_args(info, registry, std::index_sequence_for<Args...>{});

                    // Appeler la fonction C++
                    if constexpr (std::is_void_v<Ret>) {
                        std::apply(value, args_tuple);
                        return env.Undefined();
                    } else {
                        Ret result = std::apply(value, args_tuple);
                        return registry.to_napi(env, result);
                    }
                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                    return env.Undefined();
                }
            };

            return Napi::Function::New(env, callback);
        }

        std::function<Ret(Args...)> from_napi_impl(Napi::Env   env,
                                                   Napi::Value js_value) const override {
            if (!js_value.IsFunction()) {
                throw NapiConversionError("Expected a function");
            }

            // Créer un holder partagé pour la fonction JS
            auto holder = std::make_shared<JsFunctionHolder>(env, js_value.As<Napi::Function>());

            // Retourner un std::function qui appelle la fonction JS
            return [holder](Args... args) -> Ret {
                Napi::Env         env = holder->env;
                Napi::HandleScope scope(env);

                auto &registry = TypeConverterRegistry::instance();

                // Convertir les arguments C++ → JS
                std::vector<napi_value> js_args = {registry.to_napi(env, args)...};

                // Appeler la fonction JS
                Napi::Function func   = holder->func_ref.Value();
                Napi::Value    result = func.Call(js_args);

                // Convertir le résultat JS → C++
                if constexpr (!std::is_void_v<Ret>) {
                    return registry.from_napi<Ret>(env, result);
                }
            };
        }

    private:
        // template <typename... A, size_t... Is>
        // static std::tuple<A...> convert_args(const Napi::CallbackInfo &info,
        //                                      TypeConverterRegistry    &registry,
        //                                      std::index_sequence<Is...>) {
        //     return std::make_tuple(registry.from_napi<A>(info.Env(), info[Is])...);
        // }

        template <size_t... Is>
        static std::tuple<Args...> convert_args(const Napi::CallbackInfo &info,
                                                TypeConverterRegistry    &registry,
                                                std::index_sequence<Is...>) {
            return std::make_tuple(registry.from_napi<std::decay_t<Args>>(info.Env(), info[Is])...);
        }
    };

} // namespace rosetta::generators::js

#include "inline/TypeConverterRegistry.hxx"