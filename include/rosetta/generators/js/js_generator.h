// ============================================================================
// Automatic N-API/JavaScript binding generator for Rosetta introspection
// Non-intrusive design - uses Registry and ClassMetadata at runtime
// ============================================================================
#pragma once

#include <functional>
#include <memory>
#include <napi.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../type_info.h"

// Forward declarations for Rosetta types
namespace rosetta::core {
    class Any;
    class Registry;
    template <typename T> class ClassMetadata;
} // namespace rosetta::core

namespace rosetta::generators::js {

    // Type converters between C++ and JavaScript
    using CppToJsConverter = std::function<Napi::Value(Napi::Env, const core::Any &)>;
    using JsToCppConverter = std::function<core::Any(const Napi::Value &)>;

    /**
     * @brief Automatic N-API/JavaScript binding generator
     *
     * Generates JavaScript bindings at runtime using Rosetta's introspection.
     * No code generation, no inheritance, completely non-intrusive.
     *
     * @example
     * ```cpp
     * Napi::Object Init(Napi::Env env, Napi::Object exports) {
     *     JsGenerator generator(env, exports);
     *     generator.bind_classes<Person, Vehicle, Animal>();
     *     return exports;
     * }
     * NODE_API_MODULE(mymodule, Init)
     * ```
     */
    class JsGenerator {
    public:
        /**
         * @brief Constructor
         * @param env N-API environment
         * @param exports Module exports object
         */
        JsGenerator(Napi::Env env, Napi::Object exports);

        /**
         * @brief Bind a single class to JavaScript
         * @tparam Class C++ class type
         * @param custom_name Custom JavaScript name (optional)
         * @return Reference to this for chaining
         */
        template <typename Class> JsGenerator &bind_class(const std::string &custom_name = "");

        /**
         * @brief Bind multiple classes to JavaScript
         * @tparam Classes C++ class types
         * @return Reference to this for chaining
         */
        template <typename... Classes> JsGenerator &bind_classes();

        /**
         * @brief Bind a free function to JavaScript
         * @tparam Ret Return type
         * @tparam Args Argument types
         * @param name Function name in JavaScript
         * @param func Function pointer
         * @return Reference to this for chaining
         */
        template <typename Ret, typename... Args>
        JsGenerator &bind_function(const std::string &name, Ret (*func)(Args...));

        /**
         * @brief Register custom type converter using TypeInfo
         * @tparam T C++ type
         * @param to_js Converter from C++ to JS
         * @param from_js Converter from JS to C++
         * @return Reference to this for chaining
         */
        template <typename T>
        JsGenerator &register_converter(CppToJsConverter to_js, JsToCppConverter from_js);

        /**
         * @brief Register custom type converter by type_index
         * @param type_idx Type index
         * @param to_js Converter from C++ to JS
         * @param from_js Converter from JS to C++
         * @return Reference to this for chaining
         */
        JsGenerator &register_converter(std::type_index type_idx, CppToJsConverter to_js,
                                        JsToCppConverter from_js);

        /**
         * @brief Add utility functions to exports
         * @return Reference to this for chaining
         */
        JsGenerator &add_utilities();

        /**
         * @brief Get TypeInfo for a registered type
         */
        template <typename T> const TypeInfo &get_type_info() const;

        // Public for convenience
        Napi::Env    env;
        Napi::Object exports;

    private:
        // Track bound classes to avoid duplicates
        std::unordered_set<std::string> bound_classes_;

        // Type converters registry (now using type_index)
        std::unordered_map<std::type_index, CppToJsConverter> cpp_to_js_;
        std::unordered_map<std::type_index, JsToCppConverter> js_to_cpp_;

        // Store constructors for wrapped objects
        std::unordered_map<std::string, Napi::FunctionReference> constructors_;

        // Helper: Convert C++ Any to Napi::Value using TypeInfo
        Napi::Value any_to_js(Napi::Env env, const core::Any &value, const TypeInfo *type_info = nullptr);

        // Helper: Convert Napi::Value to C++ Any using TypeInfo
        core::Any js_to_any(const Napi::Value &value, const TypeInfo *type_info = nullptr);

        // Helper: Create wrapper class for a C++ type
        template <typename Class>
        Napi::Function create_wrapper_class(const std::string &class_name);

        // Helper: Bind fields to JavaScript object
        template <typename Class>
        void bind_fields(Napi::FunctionReference &ctor, const core::ClassMetadata<Class> &meta);

        // Helper: Bind methods to JavaScript object
        template <typename Class>
        void bind_methods(Napi::FunctionReference &ctor, const core::ClassMetadata<Class> &meta);

        // Initialize default converters
        void init_default_converters();

        // Helper: Get converter for a type
        CppToJsConverter get_to_js_converter(std::type_index idx) const;
        JsToCppConverter get_from_js_converter(std::type_index idx) const;
    };

    // ========================================================================
    // HELPER FUNCTIONS (can be used standalone)
    // ========================================================================

    /**
     * @brief Standalone helper to bind a class
     * @tparam T Class type
     * @param generator Generator instance
     * @param class_name JavaScript name
     */
    template <typename T>
    void bind_class(JsGenerator &generator, const std::string &class_name = "");

    /**
     * @brief Standalone helper to bind multiple classes
     * @tparam Classes Class types
     * @param generator Generator instance
     */
    template <typename... Classes> void bind_classes(JsGenerator &generator);

} // namespace rosetta::generators::js

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

/**
 * @brief Begin N-API module with a generator instance
 * Usage:
 *   BEGIN_JS_MODULE(gen) {
 *       gen.bind_classes<Person, Vehicle>();
 *   }
 *   END_JS_MODULE();
 */
#define BEGIN_JS_MODULE(generator_name)                      \
    Napi::Object Init(Napi::Env env, Napi::Object exports) { \
        rosetta::generators::js::JsGenerator generator_name(env, exports);

#define END_JS_MODULE() \
    return exports;     \
    }                   \
    NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)

/**
 * @brief Auto-bind a single class
 */
#define JS_BIND_CLASS(gen, Class) gen.bind_class<Class>(#Class)

/**
 * @brief Auto-bind multiple classes
 */
#define JS_BIND_CLASSES(gen, ...) gen.bind_classes<__VA_ARGS__>()

/**
 * @brief Auto-bind a function
 */
#define JS_BIND_FUNCTION(gen, func) gen.bind_function(#func, func)

// Include implementation
#include "inline/js_generator.hxx"