/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <rosetta/introspectable.h>
#include <sol/sol.hpp>
#include <unordered_set>

namespace rosetta {

    /**
     * @brief Automatic Sol3/Lua binding generator for introspectable classes
     * @example
     * ```cpp
     * // Usage example:
     * sol::state lua;
     * lua.open_libraries(sol::lib::base);
     *
     * LuaGenerator generator(lua);
     * generator.bind_classes<Person, Vehicle>();
     *
     * // Or bind individually
     * generator.bind_class<Person>();
     * generator.bind_class<GameObject>("GameObj");  // Custom name
     * ```
     */
    class LuaGenerator {
    public:
        explicit LuaGenerator(sol::state& lua_state)
            : lua(lua_state)
        {
        }

        /**
         * @brief Automatically bind an introspectable class to Lua
         * @tparam T The introspectable class type
         * @param class_name Optional custom class name (uses introspection name if empty)
         * @return Reference to generator for method chaining
         */
        template <typename T> LuaGenerator& bind_class(const std::string& class_name = "");

        /**
         * @brief Bind multiple classes at once
         */
        template <typename... Classes> LuaGenerator& bind_classes();

        /**
         * @brief Add module-level utility functions
         */
        LuaGenerator& add_utilities();

    private:
        sol::state& lua;
        std::unordered_set<std::string> bound_classes;

        template <typename T>
        void bind_constructors(sol::usertype<T>& user_type, const TypeInfo& type_info);

        template <typename T>
        void bind_members(sol::usertype<T>& user_type, const TypeInfo& type_info);

        template <typename T>
        void bind_methods(sol::usertype<T>& user_type, const TypeInfo& type_info);

        template <typename T> void bind_introspection_utilities(sol::usertype<T>& user_type);

        // Helper to check if method is a getter/setter
        bool is_getter_setter_method(
            const std::string& method_name, const TypeInfo& type_info) const;

        // Type conversion helpers
        sol::object convert_any_to_lua(const std::any& value, const std::string& type_name) const;
        std::any convert_lua_to_any(
            const sol::object& lua_value, const std::string& type_name) const;
    };

} // namespace rosetta

// Convenience macro
#define LUA_AUTO_BIND_CLASS(generator, ClassName) generator.bind_class<ClassName>(#ClassName)

#include "inline/lua_generator.hxx"