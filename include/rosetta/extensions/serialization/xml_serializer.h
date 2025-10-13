// ============================================================================
// rosetta/extensions/serialization/xml_serializer.hpp
//
// Sérialisation automatique en XML basée sur l'introspection
// ============================================================================
#pragma once
#include "../../core/any.h"
#include "../../core/registry.h"
#include <sstream>
#include <string>
#include <vector>

namespace rosetta::extensions {

    /**
     * @brief Sérialiseur XML automatique utilisant les métadonnées Rosetta
     */
    class XMLSerializer {
    public:
        /**
         * @brief Sérialise un objet en XML
         * @tparam T Type de l'objet
         * @param obj Objet à sérialiser
         * @param root_name Nom de l'élément racine
         * @param pretty Si true, formatte le XML avec indentation
         * @return Chaîne XML
         */
        template <typename T>
        static std::string serialize(const T &obj, const std::string &root_name = "object",
                                     bool pretty = true) {
            std::stringstream ss;

            ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            serialize_object(ss, obj, root_name, 0, pretty);

            return ss.str();
        }

        /**
         * @brief Désérialise un XML en objet
         * @tparam T Type de l'objet
         * @param xml Chaîne XML
         * @return Objet désérialisé
         * @note Implémentation simplifiée - nécessite un vrai parser XML
         */
        template <typename T> static T deserialize(const std::string &xml) {
            T obj;
            // TODO: Implémenter un vrai parser XML
            return obj;
        }

    private:
        template <typename T>
        static void serialize_object(std::stringstream &ss, const T &obj,
                                     const std::string &tag_name, int indent_level, bool pretty) {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<T>()) {
                if (pretty)
                    indent(ss, indent_level);
                ss << "<" << tag_name << "/>";
                if (pretty)
                    ss << "\n";
                return;
            }

            const auto &meta   = registry.get<T>();
            const auto &fields = meta.fields();

            if (pretty)
                indent(ss, indent_level);
            ss << "<" << tag_name;

            // Ajouter le type comme attribut
            ss << " type=\"" << meta.name() << "\"";
            ss << ">";
            if (pretty)
                ss << "\n";

            // Sérialiser les champs
            for (const auto &field_name : fields) {
                auto value = meta.get_field(const_cast<T &>(obj), field_name);
                serialize_field(ss, field_name, value, indent_level + 1, pretty);
            }

            if (pretty)
                indent(ss, indent_level);
            ss << "</" << tag_name << ">";
            if (pretty)
                ss << "\n";
        }

        static void serialize_field(std::stringstream &ss, const std::string &field_name,
                                    const core::Any &value, int indent_level, bool pretty) {
            if (pretty)
                indent(ss, indent_level);
            ss << "<" << field_name << ">";

            // Simplification - nécessite inspection du type
            ss << "value";

            ss << "</" << field_name << ">";
            if (pretty)
                ss << "\n";
        }

        static void indent(std::stringstream &ss, int level) {
            for (int i = 0; i < level; ++i) {
                ss << "  ";
            }
        }

        static std::string escape_xml(const std::string &str) {
            std::string result;
            result.reserve(str.size());

            for (char c : str) {
                switch (c) {
                case '&':
                    result += "&amp;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\'':
                    result += "&apos;";
                    break;
                default:
                    result += c;
                    break;
                }
            }

            return result;
        }
    };

} // namespace rosetta::extensions