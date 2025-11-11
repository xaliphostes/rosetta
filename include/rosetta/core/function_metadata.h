// ============================================================================
// rosetta/core/function_metadata.hpp
//
// Function metadata for Rosetta introspection system (free functions)
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

    /**
     * @brief Metadata for a free function
     * Allows introspection of free functions with any signature
     */
    class FunctionMetadata {
        std::string name_;

        // Type information
        std::vector<std::type_index> param_types_;
        std::type_index              return_type_;
        size_t                       arity_;

        // Function invoker (type-erased)
        std::function<Any(std::vector<Any>)> invoker_;

    public:
        /**
         * @brief Constructor
         * @param name Name of the function
         */
        explicit FunctionMetadata(std::string name);

        /**
         * @brief Get the function name
         */
        const std::string &name() const;

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
        template <typename T>
        static auto extract_arg(Any &any_val) -> std::remove_cv_t<std::remove_reference_t<T>>;
    };

} // namespace rosetta::core

#include "inline/function_metadata.hxx"
