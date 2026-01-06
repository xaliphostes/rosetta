// ============================================================================
// Registry for free function metadata
// ============================================================================
// UPDATED: Added support for overloaded functions
// - register_overloaded_function(): registers with is_overloaded = true
// - register_function_as(): registers with alias (different Python name)
// ============================================================================
#pragma once
#include "function_metadata.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::core {

    /**
     * @brief Registry singleton for all registered free functions
     */
    class FunctionRegistry {
        std::unordered_map<std::string, std::unique_ptr<FunctionMetadata>> functions_;

        FunctionRegistry() = default;

        // Non-copyable, non-movable
        FunctionRegistry(const FunctionRegistry &)            = delete;
        FunctionRegistry &operator=(const FunctionRegistry &) = delete;

    public:
        /**
         * @brief Get the singleton instance
         */
        static FunctionRegistry &instance();

        /**
         * @brief Register a non-overloaded free function
         * 
         * Use this for functions that have no overloads in C++.
         * Generated binding: m.def("name", &name);
         * 
         * @tparam Ret Return type
         * @tparam Args Parameter types
         * @param name Function name
         * @param ptr Function pointer
         * @return Reference to the function metadata
         */
        template <typename Ret, typename... Args>
        FunctionMetadata &register_function(const std::string &name, Ret (*ptr)(Args...));

        /**
         * @brief Register an overloaded free function (same Python name as C++ name)
         * 
         * Use this for functions that have overloads in C++.
         * Generated binding: m.def("name", static_cast<Type>(&name));
         * 
         * @tparam Ret Return type
         * @tparam Args Parameter types
         * @param name Function name
         * @param func_ptr_type_str The exact function pointer type as a string for code generation
         * @param ptr Function pointer
         * @return Reference to the function metadata
         */
        template <typename Ret, typename... Args>
        FunctionMetadata &register_overloaded_function(const std::string &name, 
                                                       const std::string &func_ptr_type_str,
                                                       Ret (*ptr)(Args...));

        /**
         * @brief Register a function with an alias (different Python name)
         * 
         * Use this when you want a different name in Python than in C++.
         * Typically used for overloaded functions where you want separate Python names.
         * Generated binding: m.def("alias", static_cast<Type>(&cpp_name));
         * 
         * @tparam Ret Return type
         * @tparam Args Parameter types
         * @param name Python binding name (alias)
         * @param cpp_name Original C++ function name
         * @param func_ptr_type_str The exact function pointer type as a string for code generation
         * @param ptr Function pointer
         * @return Reference to the function metadata
         */
        template <typename Ret, typename... Args>
        FunctionMetadata &register_function_as(const std::string &name, 
                                               const std::string &cpp_name,
                                               const std::string &func_ptr_type_str,
                                               Ret (*ptr)(Args...));

        /**
         * @brief Get metadata for a registered function
         * @param name Function name
         * @return Reference to the function metadata
         * @throws std::runtime_error if function not found
         */
        FunctionMetadata &get(const std::string &name);

        /**
         * @brief Get metadata for a registered function (const version)
         */
        const FunctionMetadata &get(const std::string &name) const;

        /**
         * @brief Check if a function is registered
         * @param name Function name
         * @return true if registered
         */
        bool has_function(const std::string &name) const;

        /**
         * @brief List all registered functions
         * @return Vector of function names
         */
        std::vector<std::string> list_functions() const;

        /**
         * @brief Get the total number of registered functions
         */
        size_t size() const;

        /**
         * @brief Clear all registered functions
         */
        void clear();

        /**
         * @brief Invoke a registered function
         * @param name Function name
         * @param args Arguments wrapped in Any
         * @return Result wrapped in Any
         */
        Any invoke(const std::string &name, std::vector<Any> args = {}) const;
    };

} // namespace rosetta::core

// ============================================================================
// Registration Macros
// ============================================================================

/**
 * @brief Register a non-overloaded free function
 * 
 * Use when the function has NO overloads in C++.
 * 
 * Example:
 *   void decimate(SurfaceMesh& mesh, unsigned int n_vertices);
 *   ROSETTA_REGISTER_FUNCTION(decimate);
 * 
 * Generated: m.def("decimate", &decimate);
 */
#define ROSETTA_REGISTER_FUNCTION(func_name) \
    rosetta::core::FunctionRegistry::instance().register_function(#func_name, &func_name)

/**
 * @brief Register an overloaded free function with explicit type
 * 
 * Use when the function HAS overloads in C++ but you want the same name in Python.
 * The FuncPtrType MUST be the exact function pointer type including const& qualifiers.
 * 
 * Example:
 *   void write(const SurfaceMesh& mesh, const std::filesystem::path& file);
 *   void write(const SurfaceMesh& mesh, const std::filesystem::path& file, IOFlags flags);
 *   
 *   // Register specific overload - note the exact type with const&
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION(write, void(*)(const SurfaceMesh&, const std::filesystem::path&));
 * 
 * Generated: m.def("write", static_cast<void(*)(const SurfaceMesh&, const std::filesystem::path&)>(&write));
 */
#define ROSETTA_REGISTER_OVERLOADED_FUNCTION(func_name, FuncPtrType) \
    rosetta::core::FunctionRegistry::instance().register_overloaded_function( \
        #func_name, #FuncPtrType, static_cast<FuncPtrType>(&func_name))

/**
 * @brief Register an overloaded free function with a different Python name (alias)
 * 
 * Use when you want a different name in Python than in C++.
 * Useful for giving overloads unique Python names.
 * The FuncPtrType MUST be the exact function pointer type including const& qualifiers.
 * 
 * Example:
 *   void triangulate(SurfaceMesh& mesh);
 *   void triangulate(SurfaceMesh& mesh, Face f);
 *   
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION(triangulate, void(*)(SurfaceMesh&));
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(triangulate, "triangulate_face", void(*)(SurfaceMesh&, Face));
 * 
 * Generated: 
 *   m.def("triangulate", static_cast<void(*)(SurfaceMesh&)>(&triangulate));
 *   m.def("triangulate_face", static_cast<void(*)(SurfaceMesh&, Face)>(&triangulate));
 */
#define ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(func_name, alias, FuncPtrType) \
    rosetta::core::FunctionRegistry::instance().register_function_as( \
        alias, #func_name, #FuncPtrType, static_cast<FuncPtrType>(&func_name))

// ============================================================================
// Short aliases for convenience
// ============================================================================
#define R_REG_FUNC(func_name) \
    ROSETTA_REGISTER_FUNCTION(func_name)

#define R_REG_FUNC_OVERLOAD(func_name, FuncPtrType) \
    ROSETTA_REGISTER_OVERLOADED_FUNCTION(func_name, FuncPtrType)

#define R_REG_FUNC_AS(func_name, alias, FuncPtrType) \
    ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(func_name, alias, FuncPtrType)

/**
 * @brief Get metadata for a registered function
 */
#define ROSETTA_GET_FUNCTION(FuncName) rosetta::core::FunctionRegistry::instance().get(#FuncName)

/**
 * @brief Check if a function is registered
 */
#define ROSETTA_HAS_FUNCTION(FuncName) \
    rosetta::core::FunctionRegistry::instance().has_function(#FuncName)

#include "inline/function_registry.hxx"