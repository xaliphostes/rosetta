/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <any>
#include <napi.h>
#include <unordered_set>

namespace rosetta {

    using CppToJsConverter = std::function<Napi::Value(Napi::Env, const std::any&)>;
    using JsToCppConverter = std::function<std::any(const Napi::Value&)>;

    /**
     * @brief Automatic N-API/JavaScript binding generator for introspectable classes
     * @example
     * ```cpp
     * // Usage example:
     * Napi::Object Init(Napi::Env env, Napi::Object exports) {
     *     rosetta::JsGenerator generator(env, exports);
     *     generator.bind_classes<Person, Vehicle>();
     *     return exports;
     * }
     * NODE_API_MODULE(jsperson, Init)
     * ```
     */
    class JsGenerator {
    public:
        JsGenerator(Napi::Env env, Napi::Object exports);

        template <typename T> JsGenerator& bind_class(const std::string& class_name = "");
        template <typename... Classes> JsGenerator& bind_classes();

        JsGenerator& add_utilities();
        JsGenerator& register_type_converter(
            const std::string&, CppToJsConverter, JsToCppConverter);

    private:
        Napi::Env env;
        Napi::Object exports;
        std::unordered_set<std::string> bound_classes;
    };

    template <typename T> void bind_class(JsGenerator&);
    template <typename... Classes> void bind_classes(JsGenerator&);

} // namespace rosetta

#define NAPI_AUTO_BIND_CLASS(generator, ClassName) generator.bind_class<ClassName>(#ClassName)

#include "inline/js_generator.hxx"