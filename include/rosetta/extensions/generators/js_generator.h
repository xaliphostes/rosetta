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

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> void bind_classes(JsGenerator &gen);

    template <typename T> void                     bind_vector_type();
    template <typename Ret, typename... Args> void register_function_converter();

} // namespace rosetta::js

// ============================================================================
// Helper Macros
// ============================================================================

#define BEGIN_JS_MODULE(module_name)                                               \
    static constexpr const char *_rosetta_module_name = #module_name;              \
    Napi::Object                 InitModule(Napi::Env env, Napi::Object exports) { \
        rosetta::js::JsGenerator generator(env, exports);

#define END_JS_MODULE() \
    return exports;     \
    }                   \
    NODE_API_MODULE(_rosetta_module_name, InitModule)

/**
 * @brief Bind a class to JavaScript (simplified macro)
 */
#define BIND_JS_CLASS(ClassName) generator.bind_class<ClassName>(#ClassName)

#define BIND_JS_VECTOR(T) rosetta::js::bind_vector_type<T>();

/**
 * @brief Register function type converter for std::function
 * Usage: BIND_JS_FUNCTION_TYPE(Point, const Point&)
 *        for std::function<Point(const Point&)>
 */
#define BIND_JS_FUNCTION_TYPE(Ret, ...) rosetta::js::register_function_converter<Ret, __VA_ARGS__>();

#include "inline/js_generator.hxx"
