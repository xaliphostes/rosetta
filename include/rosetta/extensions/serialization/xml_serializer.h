// ============================================================================
// Automatic serialization in XML based on Rosetta introspection
// ============================================================================
#pragma once
#include "../../core/any.h"
#include "../../core/registry.h"
#include <sstream>
#include <string>
#include <vector>

namespace rosetta::extensions {

    class XMLSerializer {
    public:
        template <typename T>
        static std::string serialize(const T &obj, const std::string &root_name = "object", bool pretty = true);
        template <typename T> static T deserialize(const std::string &xml);

    private:
        template <typename T>
        static void serialize_object(std::stringstream &, const T &, const std::string &, int, bool);
        static void serialize_field(std::stringstream &, const std::string &, const core::Any &, int, bool);
        static void indent(std::stringstream &, int );
        static std::string escape_xml(const std::string &);
    };

} // namespace rosetta::extensions

#include "inline/xml_serializer.hxx"