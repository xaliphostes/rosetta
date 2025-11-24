namespace rosetta::extensions {

    template <typename T>
    inline std::string XMLSerializer::serialize(const T &obj, const std::string &root_name,
                                                bool pretty) {
        std::stringstream ss;

        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        serialize_object(ss, obj, root_name, 0, pretty);

        return ss.str();
    }

    template <typename T> inline T XMLSerializer::deserialize(const std::string &xml) {
        T obj;
        // TODO: Implémenter un vrai parser XML
        return obj;
    }

    template <typename T>
    inline void XMLSerializer::serialize_object(std::stringstream &ss, const T &obj,
                                                const std::string &tag_name, int indent_level,
                                                bool pretty) {
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

    inline void XMLSerializer::serialize_field(std::stringstream &ss, const std::string &field_name,
                                               const core::Any &value, int indent_level,
                                               bool pretty) {
        if (pretty)
            indent(ss, indent_level);
        ss << "<" << field_name << ">";

        // Simplification - nécessite inspection du type
        ss << "value";

        ss << "</" << field_name << ">";
        if (pretty)
            ss << "\n";
    }

    inline void XMLSerializer::indent(std::stringstream &ss, int level) {
        for (int i = 0; i < level; ++i) {
            ss << "  ";
        }
    }

    inline std::string XMLSerializer::escape_xml(const std::string &str) {
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

} // namespace rosetta::extensions