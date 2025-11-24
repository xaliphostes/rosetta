namespace rosetta::extensions {

    template <typename T> inline std::string JSONSerializer::serialize(const T &obj, bool pretty) {
        std::stringstream ss;
        serialize_object(ss, obj, 0, pretty);
        return ss.str();
    }
    template <typename T> inline T JSONSerializer::deserialize(const std::string &json) {
        T obj;
        // TODO: Implémenter un vrai parser JSON
        // Pour l'instant, retourne un objet par défaut
        return obj;
    }

    template <typename T>
    inline void JSONSerializer::serialize_object(std::stringstream &ss, const T &obj,
                                                 int indent_level, bool pretty) {
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

    inline void JSONSerializer::serialize_value(std::stringstream &ss, const core::Any &value,
                                                int indent_level, bool pretty) {
        // Pour une implémentation complète, il faudrait inspecter le type
        // Pour l'instant, version simplifiée
        ss << "\"value\"";
    }

    inline void JSONSerializer::indent(std::stringstream &ss, int level) {
        for (int i = 0; i < level; ++i) {
            ss << "  ";
        }
    }

    template <typename T> inline std::string to_json_string(const T &value) {
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

    template <typename T> inline std::string serialize_vector(const std::vector<T> &vec) {
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