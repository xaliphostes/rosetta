// ============================================================================
// Function metadata for Rosetta introspection system (free functions)
// ============================================================================
// UPDATED: Added support for overloaded functions and aliases
// - is_overloaded(): returns true if function was registered as overloaded
// - is_aliased(): returns true if Python name differs from C++ name
// - cpp_name(): returns the original C++ function name
// ============================================================================
#pragma once
#include "any.h"
#include "demangler.h"
#include <functional>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

namespace rosetta::core {

    // ========================================================================
    // Helper trait to determine the correct return type for extract_arg
    // MUST BE DEFINED BEFORE FunctionMetadata class
    // 
    // - Non-const lvalue ref (T&): return RawType& (must return reference)
    // - Value or const ref: return RawType (by value, allows conversions)
    // ========================================================================
    namespace detail {
        template <typename T>
        struct extract_arg_return_type {
            using RawType = std::remove_cv_t<std::remove_reference_t<T>>;
            
            static constexpr bool is_nonconst_lvalue_ref = 
                std::is_lvalue_reference_v<T> && 
                !std::is_const_v<std::remove_reference_t<T>>;
            
            using type = std::conditional_t<is_nonconst_lvalue_ref, RawType&, RawType>;
        };
        
        template <typename T>
        using extract_arg_return_t = typename extract_arg_return_type<T>::type;
    } // namespace detail

    /**
     * @brief Metadata for a free function
     * Allows introspection of free functions with any signature
     */
    class FunctionMetadata {
        std::string name_;      // Registered name (may be alias like "triangulate_face")
        std::string cpp_name_;  // Original C++ function name (like "triangulate")

        // Type information
        std::vector<std::type_index> param_types_;
        std::type_index              return_type_;
        size_t                       arity_;
        
        // Flags
        bool is_overloaded_ = false;  // True if registered via register_overloaded_function
        
        // Exact function pointer type string for code generation
        // e.g., "void(*)(const pmp::SurfaceMesh&, const std::filesystem::path&)"
        // Empty for non-overloaded functions (simple &name can be used)
        std::string func_ptr_type_str_;

        // Function invoker (type-erased)
        std::function<Any(std::vector<Any>)> invoker_;

    public:
        /**
         * @brief Constructor
         * @param name Registered name of the function (used in bindings)
         * @param cpp_name Original C++ function name (defaults to name if empty)
         */
        explicit FunctionMetadata(std::string name, std::string cpp_name = "");

        /**
         * @brief Get the registered function name (used in Python bindings)
         */
        const std::string &name() const;

        /**
         * @brief Get the original C++ function name
         * For non-aliased functions, this equals name()
         * For aliased functions (e.g., "triangulate_face"), this returns the original ("triangulate")
         */
        const std::string &cpp_name() const;

        /**
         * @brief Check if this function is aliased (Python name differs from C++ name)
         */
        bool is_aliased() const;

        /**
         * @brief Check if this function was registered as overloaded
         * When true, static_cast is required in generated bindings
         */
        bool is_overloaded() const;

        /**
         * @brief Set the overloaded flag
         */
        void set_overloaded(bool value);

        /**
         * @brief Get the function pointer type string for code generation
         * Returns the exact C++ type like "void(*)(const SurfaceMesh&, const path&)"
         * Empty for non-overloaded functions
         */
        const std::string &func_ptr_type_str() const;

        /**
         * @brief Set the function pointer type string
         * Called during registration of overloaded functions
         */
        void set_func_ptr_type_str(const std::string &type_str);

        /**
         * @brief Get the number of parameters
         */
        size_t arity() const;

        /**
         * @brief Get parameter types
         */
        const std::vector<std::type_index> &param_types() const;

        /**
         * @brief Get return type
         */
        std::type_index return_type() const;

        /**
         * @brief Invoke the function with arguments
         * @param args Arguments wrapped in Any
         * @return Result wrapped in Any
         */
        Any invoke(std::vector<Any> args = {}) const;

        /**
         * @brief Register a free function
         * @tparam Ret Return type
         * @tparam Args Parameter types
         * @param ptr Function pointer
         */
        template <typename Ret, typename... Args> void register_function(Ret (*ptr)(Args...));

        /**
         * @brief Display function information
         */
        void dump(std::ostream &os) const;

    private:
        // Helper to invoke function with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_with_args(Ret (*ptr)(Args...), std::vector<Any> &args,
                                    std::index_sequence<Is...>);

        // Helper to extract argument with type conversion
        // Returns reference for non-const lvalue ref params, value otherwise
        template <typename T>
        static detail::extract_arg_return_t<T> extract_arg(Any &any_val);
    };

} // namespace rosetta::core

#include "inline/function_metadata.hxx"