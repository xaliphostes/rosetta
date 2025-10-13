// ============================================================================
// rosetta/extensions/serialization/json_serializer.hpp
//
// Sérialisation automatique en JSON basée sur l'introspection
// ============================================================================
#pragma once
#include "../../core/any.h"
#include "../../core/registry.h"
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace rosetta::extensions {

    /**
     * @brief Sérialiseur JSON automatique utilisant les métadonnées Rosetta
     */
    class JSONSerializer {
    public:
        /**
         * @brief Sérialise un objet en JSON
         * @tparam T Type de l'objet
         * @param obj Objet à sérialiser
         * @param pretty Si true, formatte le JSON avec indentation
         * @return Chaîne JSON
         */
        template <typename T> static std::string serialize(const T &obj, bool pretty = true) {
            std::stringstream ss;
            serialize_object(ss, obj, 0, pretty);
            return ss.str();
        }

        /**
         * @brief Désérialise un JSON en objet
         * @tparam T Type de l'objet
         * @param json Chaîne JSON
         * @return Objet désérialisé
         * @note Implémentation simplifiée - nécessite un vrai parser JSON
         */
        template <typename T> static T deserialize(const std::string &json) {
            T obj;
            // TODO: Implémenter un vrai parser JSON
            // Pour l'instant, retourne un objet par défaut
            return obj;
        }

    private:
        template <typename T>
        static void serialize_object(std::stringstream &ss, const T &obj, int indent_level,
                                     bool pretty) {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<T>()) {
                ss << "{}";
                return;
            }

            const auto &meta   = registry.get<T>();
            const auto &fields = meta.fields();

            ss << "{";
            if (pretty)
                ss << "\n";

            bool first = true;
            for (const auto &field_name : fields) {
                if (!first) {
                    ss << ",";
                    if (pretty)
                        ss << "\n";
                }
                first = false;

                if (pretty) {
                    indent(ss, indent_level + 1);
                }

                ss << "\"" << field_name << "\": ";

                // Récupérer la valeur du champ
                auto value = meta.get_field(const_cast<T &>(obj), field_name);
                serialize_value(ss, value, indent_level + 1, pretty);
            }

            if (pretty) {
                ss << "\n";
                indent(ss, indent_level);
            }
            ss << "}";
        }

        static void serialize_value(std::stringstream &ss, const core::Any &value, int indent_level,
                                    bool pretty) {
            // Pour une implémentation complète, il faudrait inspecter le type
            // Pour l'instant, version simplifiée
            ss << "\"value\"";
        }

        static void indent(std::stringstream &ss, int level) {
            for (int i = 0; i < level; ++i) {
                ss << "  ";
            }
        }
    };

    /**
     * @brief Helper pour sérialiser des types primitifs
     */
    template <typename T> std::string to_json_string(const T &value) {
        if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + value + "\"";
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(value);
        } else {
            return "null";
        }
    }

    /**
     * @brief Sérialisation de std::vector
     */
    template <typename T> std::string serialize_vector(const std::vector<T> &vec) {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0)
                ss << ", ";
            ss << to_json_string(vec[i]);
        }
        ss << "]";
        return ss.str();
    }

} // namespace rosetta::extensions