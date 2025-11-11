// ============================================================================
// JavaScript/Node.js binding generator using Rosetta introspection
// No inheritance or wrapping required - pure non-intrusive approach
// ============================================================================
#pragma once

#include <functional>
#include <napi.h>
#include <rosetta/rosetta.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::js {

    /**
     * @brief Main generator class for JavaScript bindings
     */
    class JsGenerator {
    public:
        JsGenerator(Napi::Env env, Napi::Object exports);

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param js_name Optional JavaScript name (uses C++ name if empty)
         */
        template <typename T> JsGenerator &bind_class(const std::string &js_name = "");

        /**
         * @brief Add utility functions
         */
        JsGenerator &add_utilities();

    private:
        Napi::Env    env_;
        Napi::Object exports_;
    };

// ============================================================================
// Helper Macros
// ============================================================================

/**
 * @brief Bind a class to JavaScript (simplified macro)
 */
#define ROSETTA_BIND_JS_CLASS(Generator, ClassName) Generator.bind_class<ClassName>(#ClassName)

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> void bind_classes(JsGenerator &gen);

} // namespace rosetta::js

#define BEGIN_JS_MODULE(module_name)                   \
    Napi::Object InitModule(Napi::Env env, Napi::Object exports) { \
        rosetta::js::JsGenerator generator(env, exports);

#define END_JS_MODULE(module_name) \
    return exports;                \
    }                              \
    NODE_API_MODULE(module_name, InitModule)

#include "inline/js_generator.hxx"