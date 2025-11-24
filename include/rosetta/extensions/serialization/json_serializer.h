// ============================================================================
// Automatique serialization in JSON based on Rosetta introspection
// ============================================================================
#pragma once
#include "../../core/any.h"
#include "../../core/registry.h"
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace rosetta::extensions {

    class JSONSerializer {
    public:
        template <typename T> static std::string serialize(const T &obj, bool pretty = true);
        template <typename T> static T deserialize(const std::string &json);

    private:
        template <typename T>
        static void serialize_object(std::stringstream &ss, const T &obj, int indent_level, bool pretty);
        static void serialize_value(std::stringstream &ss, const core::Any &value, int indent_level, bool pretty);
        static void indent(std::stringstream &ss, int level);
    };

    template <typename T> std::string to_json_string(const T &value);
    template <typename T> std::string serialize_vector(const std::vector<T> &vec);

} // namespace rosetta::extensions

#include "inline/json_serializer.hxx"