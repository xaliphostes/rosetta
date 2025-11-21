// ============================================================================
// Registry for free function metadata
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
         * @brief Register a free function
         * @tparam Ret Return type
         * @tparam Args Parameter types
         * @param name Function name
         * @param ptr Function pointer
         * @return Reference to the function metadata
         */
        template <typename Ret, typename... Args>
        FunctionMetadata &register_function(const std::string &name, Ret (*ptr)(Args...));

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

#include "inline/function_registry.hxx"
