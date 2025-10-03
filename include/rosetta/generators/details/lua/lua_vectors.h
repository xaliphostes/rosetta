/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <rosetta/generators/lua.h>
#include <rosetta/type_registry.h>
#include <sol/sol.hpp>
#include <type_traits>
#include <typeinfo>

namespace rosetta {

    // ============================================================================
    // Lua-specific type conversion helpers
    // ============================================================================

    /**
     * @brief Register vector type converter for Lua using automatic type name
     * @tparam T Element type of the vector
     * @param lua The Lua state
     *
     * Sol3 automatically handles std::vector for most basic types,
     * but explicit registration provides better control.
     */
    template <typename T> inline void registerVectorType(sol::state& lua)
    {
        // Sol3 has some automatic conversions, but explicit is better
        std::string type_name = std::string("vector_") + typeid(T).name();

        // Register as new usertype
        lua.new_usertype<std::vector<T>>(
            type_name, sol::constructors<std::vector<T>(), std::vector<T>(size_t)>(),

            // Array-like access
            sol::meta_function::length, [](const std::vector<T>& v) { return v.size(); },
            sol::meta_function::index,
            [](const std::vector<T>& v, size_t i) -> T {
                if (i < 1 || i > v.size()) {
                    throw std::out_of_range("Index out of range");
                }
                return v[i - 1]; // Lua is 1-indexed
            },
            sol::meta_function::new_index,
            [](std::vector<T>& v, size_t i, T value) {
                if (i < 1 || i > v.size()) {
                    throw std::out_of_range("Index out of range");
                }
                v[i - 1] = value; // Lua is 1-indexed
            },

            // Methods
            "push_back", &std::vector<T>::push_back, "size", &std::vector<T>::size, "clear",
            &std::vector<T>::clear, "empty", &std::vector<T>::empty);
    }

    /**
     * @brief Register type alias for Lua
     * @tparam AliasType The type alias (e.g., Vertices)
     * @tparam ElementType The element type (e.g., double)
     * @param lua The Lua state
     *
     * This creates a Lua usertype for the alias that behaves like a vector
     */
    template <typename AliasType, typename ElementType>
    inline void registerTypeAlias(sol::state& lua)
    {
        std::string type_name = "alias_" + std::string(typeid(AliasType).name());

        lua.new_usertype<AliasType>(
            type_name, sol::constructors<AliasType(), AliasType(size_t)>(),

            sol::meta_function::length, [](const AliasType& v) { return v.size(); },
            sol::meta_function::index,
            [](const AliasType& v, size_t i) -> ElementType {
                if (i < 1 || i > v.size())
                    return ElementType {};
                return v[i - 1];
            },
            sol::meta_function::new_index,
            [](AliasType& v, size_t i, ElementType value) {
                if (i >= 1 && i <= v.size()) {
                    v[i - 1] = value;
                }
            },

            "push_back", &AliasType::push_back, "size", &AliasType::size, "clear",
            &AliasType::clear);
    }

    /**
     * @brief Register all common vector types for Lua
     * @param lua The Lua state
     */
    inline void registerCommonVectorTypes(sol::state& lua)
    {
        // Integer types
        registerVectorType<int>(lua);
        registerVectorType<unsigned int>(lua);
        registerVectorType<size_t>(lua);

        // Floating point types
        registerVectorType<float>(lua);
        registerVectorType<double>(lua);

        // Other types
        registerVectorType<bool>(lua);
        registerVectorType<std::string>(lua);
    }

    /**
     * @brief Helper to convert Lua table to std::vector
     * @tparam T Element type
     * @param lua_table The Lua table
     * @return std::vector<T>
     */
    template <typename T> inline std::vector<T> tableToVector(const sol::table& lua_table)
    {
        std::vector<T> vec;
        for (size_t i = 1; i <= lua_table.size(); ++i) { // Lua is 1-indexed
            vec.push_back(lua_table[i]);
        }
        return vec;
    }

    /**
     * @brief Helper to convert std::vector to Lua table
     * @tparam T Element type
     * @param lua The Lua state
     * @param vec The vector to convert
     * @return sol::table
     */
    template <typename T>
    inline sol::table vectorToTable(sol::state& lua, const std::vector<T>& vec)
    {
        sol::table table = lua.create_table();
        for (size_t i = 0; i < vec.size(); ++i) {
            table[i + 1] = vec[i]; // Lua is 1-indexed
        }
        return table;
    }

} // namespace rosetta

// ============================================================================
// USAGE GUIDE FOR LUA
// ============================================================================
/*

BASIC USAGE:
-----------

    #include <rosetta/generators/lua.h>
    #include <rosetta/generators/lua_vector_helpers.h>

    using Vertices = std::vector<double>;
    using Triangles = std::vector<size_t>;

    REGISTER_TYPE_ALIAS_MANGLED(Vertices);
    REGISTER_TYPE_ALIAS_MANGLED(Triangles);

    int main() {
        sol::state lua;
        lua.open_libraries(sol::lib::base);

        // Register vector types for Lua
        rosetta::registerTypeAlias<Vertices, double>(lua);
        rosetta::registerTypeAlias<Triangles, size_t>(lua);

        // Bind your classes
        rosetta::LuaGenerator generator(lua);
        generator.bind_class<Surface>();

        // Run Lua script
        lua.script_file("test.lua");
    }


LUA USAGE:
---------

    -- test.lua
    local s = Surface.new({1.0, 2.0, 3.0}, {0, 1, 2})

    local verts = s:vertices()
    print("Vertices:", #verts)  -- Length: 3
    print("First:", verts[1])    -- Lua is 1-indexed: 1.0

    -- Modify
    verts[1] = 10.0
    verts:push_back(4.0)


REGISTER ALL COMMON TYPES:
--------------------------

    int main() {
        sol::state lua;
        lua.open_libraries(sol::lib::base);

        // Register all common vector types at once
        rosetta::registerCommonVectorTypes(lua);

        rosetta::LuaGenerator generator(lua);
        generator.bind_class<Surface>();
    }


CUSTOM TYPES:
------------

For custom types like Point3D:

    struct Point3D {
        double x, y, z;
    };

    int main() {
        sol::state lua;
        lua.open_libraries(sol::lib::base);

        // Register Point3D
        lua.new_usertype<Point3D>("Point3D",
            sol::constructors<Point3D(), Point3D(double, double, double)>(),
            "x", &Point3D::x,
            "y", &Point3D::y,
            "z", &Point3D::z
        );

        // Register vector of Point3D
        rosetta::registerVectorType<Point3D>(lua);

        rosetta::LuaGenerator generator(lua);
        generator.bind_class<Mesh>();
    }

    -- Lua usage:
    -- local points = {
    --     Point3D.new(0,0,0),
    --     Point3D.new(1,1,1)
    -- }
    -- local mesh = Mesh.new(points)


HELPER FUNCTIONS:
----------------

Convert between Lua tables and C++ vectors:

    // In C++ callback
    auto lua_callback = [](sol::table t) {
        auto vec = rosetta::tableToVector<double>(t);
        // Process vec...
        return rosetta::vectorToTable(lua, result_vec);
    };


COMPARISON: JavaScript vs Python vs Lua
---------------------------------------

JavaScript:
    // Explicit converters needed
    rosetta::registerTypeAlias<Vertices, double>(generator);

Python:
    // Automatic via pybind11 (no registration needed)
    generator.bind_class<Surface>();

Lua:
    // Explicit registration for better control
    rosetta::registerTypeAlias<Vertices, double>(lua);
    generator.bind_class<Surface>();

*/