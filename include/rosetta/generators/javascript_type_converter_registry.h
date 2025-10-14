// ============================================================================
// rosetta/generators/type_converter_registry.hpp
//
// Registry pour convertir Any <-> NAPI de manière extensible (C++ wrapper)
// ============================================================================
#pragma once
<<<<<<< HEAD
#include <napi.h>
#include "../core/any.h"
#include <functional>
#include <memory>
=======
#include "../core/any.h"
#include <functional>
#include <memory>
#include <napi.h>
>>>>>>> e2cebf3aea7fc93e17e96fcec02269b9119b98c8
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rosetta::generators {

    /**
     * @brief Registry pour les conversions de types Any <-> Node-API
     *
     * Permet d'enregistrer des convertisseurs pour chaque type C++
     * de manière extensible et type-safe
     */
    class TypeConverterRegistry {
    public:
        /**
         * @brief Fonction de conversion Any -> Napi::Value
         */
        using ToNapiConverter = std::function<Napi::Value(Napi::Env, const core::Any &)>;

        /**
         * @brief Fonction de conversion Napi::Value -> Any
         */
        using FromNapiConverter = std::function<core::Any(Napi::Env, const Napi::Value &)>;

    private:
        std::unordered_map<std::type_index, ToNapiConverter>   to_napi_converters_;
        std::unordered_map<std::type_index, FromNapiConverter> from_napi_converters_;
        std::unordered_map<std::string, std::type_index *>     type_name_to_index_;
        std::vector<std::unique_ptr<std::type_index>> type_indices_; // Ownership des type_index

        TypeConverterRegistry() { register_builtin_converters(); }

        // Non-copiable
        TypeConverterRegistry(const TypeConverterRegistry &)            = delete;
        TypeConverterRegistry &operator=(const TypeConverterRegistry &) = delete;

    public:
        /**
         * @brief Obtient l'instance singleton
         */
        static TypeConverterRegistry &instance() {
            static TypeConverterRegistry registry;
            return registry;
        }

        /**
         * @brief Enregistre un convertisseur pour un type
         * @tparam T Type C++
         * @param to_napi Fonction de conversion T -> Napi::Value
         * @param from_napi Fonction de conversion Napi::Value -> T
         */
        template <typename T>
        void register_converter(ToNapiConverter to_napi, FromNapiConverter from_napi) {
            std::type_index idx(typeid(T));
            to_napi_converters_[idx]   = std::move(to_napi);
            from_napi_converters_[idx] = std::move(from_napi);

            // Stocker le type_index avec ownership
            auto type_idx_ptr                     = std::make_unique<std::type_index>(typeid(T));
            type_name_to_index_[typeid(T).name()] = type_idx_ptr.get();
            type_indices_.push_back(std::move(type_idx_ptr));
        }

        /**
         * @brief Convertit Any -> Napi::Value
         * @param env Environnement NAPI
         * @param value Valeur Any à convertir
         * @return Napi::Value ou undefined si type non supporté
         */
        Napi::Value any_to_napi(Napi::Env env, const core::Any &value) const {
            if (!value.has_value()) {
                return env.Undefined();
            }

            // Essayer de trouver un convertisseur par nom de type
            std::string type_name = value.type_name();
            auto        name_it   = type_name_to_index_.find(type_name);

            if (name_it != type_name_to_index_.end()) {
                auto conv_it = to_napi_converters_.find(*(name_it->second));
                if (conv_it != to_napi_converters_.end()) {
                    return conv_it->second(env, value);
                }
            }

            // Fallback: essayer chaque convertisseur jusqu'à ce qu'un fonctionne
            for (const auto &[type_idx, converter] : to_napi_converters_) {
                try {
                    return converter(env, value);
                } catch (...) {
                    // Ce type ne correspond pas, continuer
                }
            }

            // Aucun convertisseur trouvé
            return env.Undefined();
        }

        /**
         * @brief Convertit Napi::Value -> Any
         * @param env Environnement NAPI
         * @param value Valeur NAPI à convertir
         * @return Any contenant la valeur convertie
         */
        core::Any napi_to_any(Napi::Env env, const Napi::Value &value) const {
            if (value.IsUndefined() || value.IsNull()) {
                return core::Any();
            }

            if (value.IsBoolean()) {
                return core::Any(value.As<Napi::Boolean>().Value());
            }

            if (value.IsNumber()) {
                return core::Any(value.As<Napi::Number>().DoubleValue());
            }

            if (value.IsString()) {
                return core::Any(value.As<Napi::String>().Utf8Value());
            }

            if (value.IsObject()) {
                // Pour les objets, on pourrait unwrap les instances C++
                // Pour l'instant, retourner un any vide
                return core::Any();
            }

            return core::Any();
        }

        /**
         * @brief Vérifie si un type est supporté
         * @tparam T Type à vérifier
         */
        template <typename T> bool has_converter() const {
            std::type_index idx(typeid(T));
            return to_napi_converters_.find(idx) != to_napi_converters_.end();
        }

        /**
         * @brief Liste tous les types enregistrés
         */
        std::vector<std::string> list_types() const {
            std::vector<std::string> types;
            for (const auto &[name, idx_ptr] : type_name_to_index_) {
                types.push_back(name);
            }
            return types;
        }

    private:
        /**
         * @brief Enregistre les convertisseurs pour les types built-in
         */
        void register_builtin_converters() {
            // bool
            register_converter<bool>(
                // To NAPI
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    bool b = const_cast<core::Any &>(value).as<bool>();
                    return Napi::Boolean::New(env, b);
                },
                // From NAPI
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Boolean>().Value());
                });

            // int
            register_converter<int>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    int i = const_cast<core::Any &>(value).as<int>();
                    return Napi::Number::New(env, i);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().Int32Value());
                });

            // int32_t
            register_converter<int32_t>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    int32_t i = const_cast<core::Any &>(value).as<int32_t>();
                    return Napi::Number::New(env, i);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().Int32Value());
                });

            // int64_t
            register_converter<int64_t>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    int64_t i = const_cast<core::Any &>(value).as<int64_t>();
                    return Napi::Number::New(env, static_cast<double>(i));
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().Int64Value());
                });

            // uint32_t
            register_converter<uint32_t>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    uint32_t u = const_cast<core::Any &>(value).as<uint32_t>();
                    return Napi::Number::New(env, u);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().Uint32Value());
                });

            // float
            register_converter<float>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    float f = const_cast<core::Any &>(value).as<float>();
                    return Napi::Number::New(env, static_cast<double>(f));
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().FloatValue());
                });

            // double
            register_converter<double>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    double d = const_cast<core::Any &>(value).as<double>();
                    return Napi::Number::New(env, d);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::Number>().DoubleValue());
                });

            // std::string
            register_converter<std::string>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    const std::string &s = const_cast<core::Any &>(value).as<std::string>();
                    return Napi::String::New(env, s);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    return core::Any(value.As<Napi::String>().Utf8Value());
                });

            // const char*
            register_converter<const char *>(
                [](Napi::Env env, const core::Any &value) -> Napi::Value {
                    const char *s = const_cast<core::Any &>(value).as<const char *>();
                    return Napi::String::New(env, s);
                },
                [](Napi::Env env, const Napi::Value &value) -> core::Any {
                    // Note: retourne string, pas const char*
                    return core::Any(value.As<Napi::String>().Utf8Value());
                });
        }
    };

    /**
     * @brief Helper pour enregistrer facilement des convertisseurs custom
     */
    template <typename T>
    void register_napi_converter(std::function<Napi::Value(Napi::Env, const T &)> to_napi,
                                 std::function<T(Napi::Env, const Napi::Value &)> from_napi) {
        TypeConverterRegistry::instance().register_converter<T>(
            // Wrapper to_napi
            [to_napi](Napi::Env env, const core::Any &value) -> Napi::Value {
                const T &typed_value = const_cast<core::Any &>(value).as<T>();
                return to_napi(env, typed_value);
            },
            // Wrapper from_napi
            [from_napi](Napi::Env env, const Napi::Value &value) -> core::Any {
                T typed_value = from_napi(env, value);
                return core::Any(typed_value);
            });
    }

} // namespace rosetta::generators

// ============================================================================
// EXEMPLE D'UTILISATION
// ============================================================================

/*

#include "rosetta/generators/type_converter_registry.hpp"

// 1. Enregistrer un type custom
struct Color {
    uint8_t r, g, b, a;
};

void register_color_converter() {
    using namespace rosetta::generators;

    register_napi_converter<Color>(
        // To NAPI: convertir Color en objet JavaScript
        [](Napi::Env env, const Color& color) -> Napi::Value {
            Napi::Object obj = Napi::Object::New(env);

            obj.Set("r", Napi::Number::New(env, color.r));
            obj.Set("g", Napi::Number::New(env, color.g));
            obj.Set("b", Napi::Number::New(env, color.b));
            obj.Set("a", Napi::Number::New(env, color.a));

            return obj;
        },
        // From NAPI: convertir objet JavaScript en Color
        [](Napi::Env env, const Napi::Value& value) -> Color {
            Napi::Object obj = value.As<Napi::Object>();

            Color color = {0, 0, 0, 255};

            if (obj.Has("r")) color.r =
static_cast<uint8_t>(obj.Get("r").As<Napi::Number>().Uint32Value()); if (obj.Has("g")) color.g =
static_cast<uint8_t>(obj.Get("g").As<Napi::Number>().Uint32Value()); if (obj.Has("b")) color.b =
static_cast<uint8_t>(obj.Get("b").As<Napi::Number>().Uint32Value()); if (obj.Has("a")) color.a =
static_cast<uint8_t>(obj.Get("a").As<Napi::Number>().Uint32Value());

            return color;
        }
    );
}

// 2. Utiliser le registry
void example() {
    Napi::Env env = ...; // Votre environnement NAPI

    auto& registry = rosetta::generators::TypeConverterRegistry::instance();

    // Convertir C++ -> JavaScript
    Color red = {255, 0, 0, 255};
    rosetta::core::Any any_color(red);
    Napi::Value js_color = registry.any_to_napi(env, any_color);

    // Convertir JavaScript -> C++
    rosetta::core::Any converted = registry.napi_to_any(env, js_color);
    Color& cpp_color = converted.as<Color>();
}

// 3. Exemple avec std::vector
void register_vector_converter() {
    using namespace rosetta::generators;

    register_napi_converter<std::vector<double>>(
        // To NAPI: convertir vector en Array JavaScript
        [](Napi::Env env, const std::vector<double>& vec) -> Napi::Value {
            Napi::Array arr = Napi::Array::New(env, vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                arr[i] = Napi::Number::New(env, vec[i]);
            }
            return arr;
        },
        // From NAPI: convertir Array JavaScript en vector
        [](Napi::Env env, const Napi::Value& value) -> std::vector<double> {
            Napi::Array arr = value.As<Napi::Array>();
            std::vector<double> vec;
            vec.reserve(arr.Length());

            for (uint32_t i = 0; i < arr.Length(); ++i) {
                Napi::Value element = arr[i];
                if (element.IsNumber()) {
                    vec.push_back(element.As<Napi::Number>().DoubleValue());
                }
            }
            return vec;
        }
    );
}

// 4. Exemple avec std::map
void register_map_converter() {
    using namespace rosetta::generators;

    register_napi_converter<std::map<std::string, int>>(
        // To NAPI: convertir map en Object JavaScript
        [](Napi::Env env, const std::map<std::string, int>& map) -> Napi::Value {
            Napi::Object obj = Napi::Object::New(env);
            for (const auto& [key, value] : map) {
                obj.Set(key, Napi::Number::New(env, value));
            }
            return obj;
        },
        // From NAPI: convertir Object JavaScript en map
        [](Napi::Env env, const Napi::Value& value) -> std::map<std::string, int> {
            Napi::Object obj = value.As<Napi::Object>();
            std::map<std::string, int> map;

            Napi::Array props = obj.GetPropertyNames();
            for (uint32_t i = 0; i < props.Length(); ++i) {
                std::string key = props.Get(i).As<Napi::String>().Utf8Value();
                Napi::Value val = obj.Get(key);
                if (val.IsNumber()) {
                    map[key] = val.As<Napi::Number>().Int32Value();
                }
            }
            return map;
        }
    );
}

*/