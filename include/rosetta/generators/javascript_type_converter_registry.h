// ============================================================================
// rosetta/generators/type_converter_registry.hpp
//
// Registry pour convertir Any <-> NAPI de manière extensible
// ============================================================================
#pragma once
#include "../core/any.h"
#include <functional>
#include <memory>
#include <node_api.h>
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
         * @brief Fonction de conversion Any -> napi_value
         */
        using ToNapiConverter = std::function<napi_value(napi_env, const core::Any &)>;

        /**
         * @brief Fonction de conversion napi_value -> Any
         */
        using FromNapiConverter = std::function<core::Any(napi_env, napi_value)>;

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
         * @param to_napi Fonction de conversion T -> napi_value
         * @param from_napi Fonction de conversion napi_value -> T
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
         * @brief Convertit Any -> napi_value
         * @param env Environnement NAPI
         * @param value Valeur Any à convertir
         * @return napi_value ou undefined si type non supporté
         */
        napi_value any_to_napi(napi_env env, const core::Any &value) const {
            if (!value.has_value()) {
                napi_value result;
                napi_get_undefined(env, &result);
                return result;
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
            napi_value result;
            napi_get_undefined(env, &result);
            return result;
        }

        /**
         * @brief Convertit napi_value -> Any
         * @param env Environnement NAPI
         * @param value Valeur NAPI à convertir
         * @return Any contenant la valeur convertie
         */
        core::Any napi_to_any(napi_env env, napi_value value) const {
            napi_valuetype type;
            napi_typeof(env, value, &type);

            switch (type) {
            case napi_undefined:
            case napi_null:
                return core::Any();

            case napi_boolean: {
                bool b;
                napi_get_value_bool(env, value, &b);
                return core::Any(b);
            }

            case napi_number: {
                double d;
                napi_get_value_double(env, value, &d);
                return core::Any(d);
            }

            case napi_string: {
                size_t length;
                napi_get_value_string_utf8(env, value, nullptr, 0, &length);
                std::string str(length, '\0');
                napi_get_value_string_utf8(env, value, &str[0], length + 1, nullptr);
                return core::Any(str);
            }

            case napi_object: {
                // Pour les objets, on pourrait unwrap les instances C++
                // Pour l'instant, retourner un any vide
                return core::Any();
            }

            default:
                return core::Any();
            }
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
                [](napi_env env, const core::Any &value) -> napi_value {
                    bool       b = const_cast<core::Any &>(value).as<bool>();
                    napi_value result;
                    napi_get_boolean(env, b, &result);
                    return result;
                },
                // From NAPI
                [](napi_env env, napi_value value) -> core::Any {
                    bool b;
                    napi_get_value_bool(env, value, &b);
                    return core::Any(b);
                });

            // int
            register_converter<int>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    int        i = const_cast<core::Any &>(value).as<int>();
                    napi_value result;
                    napi_create_int32(env, i, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    int32_t i;
                    napi_get_value_int32(env, value, &i);
                    return core::Any(static_cast<int>(i));
                });

            // int32_t
            register_converter<int32_t>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    int32_t    i = const_cast<core::Any &>(value).as<int32_t>();
                    napi_value result;
                    napi_create_int32(env, i, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    int32_t i;
                    napi_get_value_int32(env, value, &i);
                    return core::Any(i);
                });

            // int64_t
            register_converter<int64_t>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    int64_t    i = const_cast<core::Any &>(value).as<int64_t>();
                    napi_value result;
                    napi_create_int64(env, i, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    int64_t i;
                    napi_get_value_int64(env, value, &i);
                    return core::Any(i);
                });

            // uint32_t
            register_converter<uint32_t>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    uint32_t   u = const_cast<core::Any &>(value).as<uint32_t>();
                    napi_value result;
                    napi_create_uint32(env, u, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    uint32_t u;
                    napi_get_value_uint32(env, value, &u);
                    return core::Any(u);
                });

            // float
            register_converter<float>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    float      f = const_cast<core::Any &>(value).as<float>();
                    napi_value result;
                    napi_create_double(env, static_cast<double>(f), &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    double d;
                    napi_get_value_double(env, value, &d);
                    return core::Any(static_cast<float>(d));
                });

            // double
            register_converter<double>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    double     d = const_cast<core::Any &>(value).as<double>();
                    napi_value result;
                    napi_create_double(env, d, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    double d;
                    napi_get_value_double(env, value, &d);
                    return core::Any(d);
                });

            // std::string
            register_converter<std::string>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    const std::string &s = const_cast<core::Any &>(value).as<std::string>();
                    napi_value         result;
                    napi_create_string_utf8(env, s.c_str(), s.length(), &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    size_t length;
                    napi_get_value_string_utf8(env, value, nullptr, 0, &length);
                    std::string str(length, '\0');
                    napi_get_value_string_utf8(env, value, &str[0], length + 1, nullptr);
                    return core::Any(str);
                });

            // const char*
            register_converter<const char *>(
                [](napi_env env, const core::Any &value) -> napi_value {
                    const char *s = const_cast<core::Any &>(value).as<const char *>();
                    napi_value  result;
                    napi_create_string_utf8(env, s, NAPI_AUTO_LENGTH, &result);
                    return result;
                },
                [](napi_env env, napi_value value) -> core::Any {
                    size_t length;
                    napi_get_value_string_utf8(env, value, nullptr, 0, &length);
                    std::string str(length, '\0');
                    napi_get_value_string_utf8(env, value, &str[0], length + 1, nullptr);
                    return core::Any(str); // Note: retourne string, pas const char*
                });
        }
    };

    /**
     * @brief Helper pour enregistrer facilement des convertisseurs custom
     */
    template <typename T>
    void register_napi_converter(std::function<napi_value(napi_env, const T &)> to_napi,
                                 std::function<T(napi_env, napi_value)>         from_napi) {
        TypeConverterRegistry::instance().register_converter<T>(
            // Wrapper to_napi
            [to_napi](napi_env env, const core::Any &value) -> napi_value {
                const T &typed_value = const_cast<core::Any &>(value).as<T>();
                return to_napi(env, typed_value);
            },
            // Wrapper from_napi
            [from_napi](napi_env env, napi_value value) -> core::Any {
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
        [](napi_env env, const Color& color) -> napi_value {
            napi_value obj;
            napi_create_object(env, &obj);

            napi_value r, g, b, a;
            napi_create_uint32(env, color.r, &r);
            napi_create_uint32(env, color.g, &g);
            napi_create_uint32(env, color.b, &b);
            napi_create_uint32(env, color.a, &a);

            napi_set_named_property(env, obj, "r", r);
            napi_set_named_property(env, obj, "g", g);
            napi_set_named_property(env, obj, "b", b);
            napi_set_named_property(env, obj, "a", a);

            return obj;
        },
        // From NAPI: convertir objet JavaScript en Color
        [](napi_env env, napi_value value) -> Color {
            Color color = {0, 0, 0, 255};

            napi_value r, g, b, a;
            napi_get_named_property(env, value, "r", &r);
            napi_get_named_property(env, value, "g", &g);
            napi_get_named_property(env, value, "b", &b);
            napi_get_named_property(env, value, "a", &a);

            uint32_t r_val, g_val, b_val, a_val;
            napi_get_value_uint32(env, r, &r_val);
            napi_get_value_uint32(env, g, &g_val);
            napi_get_value_uint32(env, b, &b_val);
            napi_get_value_uint32(env, a, &a_val);

            color.r = static_cast<uint8_t>(r_val);
            color.g = static_cast<uint8_t>(g_val);
            color.b = static_cast<uint8_t>(b_val);
            color.a = static_cast<uint8_t>(a_val);

            return color;
        }
    );
}

// 2. Utiliser le registry
void example() {
    napi_env env = ...; // Votre environnement NAPI

    auto& registry = rosetta::generators::TypeConverterRegistry::instance();

    // Convertir C++ -> JavaScript
    Color red = {255, 0, 0, 255};
    rosetta::core::Any any_color(red);
    napi_value js_color = registry.any_to_napi(env, any_color);

    // Convertir JavaScript -> C++
    rosetta::core::Any converted = registry.napi_to_any(env, js_color);
    Color& cpp_color = converted.as<Color>();
}

*/