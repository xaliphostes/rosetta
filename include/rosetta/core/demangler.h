// ============================================================================
// Utilities for demangling type names and creating readable type strings
// ============================================================================
#pragma once
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace rosetta::core {

    /**
     * @brief Demangle a C++ type name
     * @param name Mangled name from typeid().name()
     * @return Demangled human-readable name
     */
    std::string demangle(const char *name);

    /**
     * @brief Demangle a type_index
     */
    inline std::string demangle(const std::type_index &ti);

    /**
     * @brief Clean up type names for better readability
     * @param type_name Demangled type name
     * @return Cleaned up type name
     */
    inline std::string cleanup_type_name(const std::string &type_name);

    /**
     * @brief Get a human-readable type name
     * @tparam T Type to get name for
     * @return Readable type name
     */
    template <typename T> std::string get_readable_type_name();

    /**
     * @brief Get a human-readable type name from type_index
     */
    inline std::string get_readable_type_name(const std::type_index &ti);

    /**
     * @brief Type name registry for custom type aliases
     *
     * This allows registering custom readable names for types
     */
    class TypeNameRegistry {
        std::unordered_map<std::type_index, std::string> type_names_;

        TypeNameRegistry() = default;

    public:
        static TypeNameRegistry &instance();

        /**
         * @brief Register a custom name for a type
         */
        template <typename T> void register_name(const std::string &name);

        /**
         * @brief Get the registered name, or demangle if not registered
         */
        std::string get_name(const std::type_index &ti) const;

        /**
         * @brief Check if a type has a custom registered name
         */
        bool has_custom_name(const std::type_index &ti) const;

        void clear();
    };

} // namespace rosetta::core

#include "inline/demangler.hxx"
