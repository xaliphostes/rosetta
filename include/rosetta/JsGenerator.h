#pragma once
#include <any>
#include <napi.h>
#include <unordered_set>

namespace rosetta {

    using CppToJsConverter = std::function<Napi::Value(Napi::Env, const std::any&)>;
    using JsToCppConverter = std::function<std::any(const Napi::Value&)>;

    /**
     * @brief Binding generator
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

} // namespace rosetta

#define NAPI_AUTO_BIND_CLASS(generator, ClassName) generator.bind_class<ClassName>(#ClassName)

#include "inline/JsGenerator.hxx"