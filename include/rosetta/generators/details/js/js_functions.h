/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "js_generator.h"
#include <rosetta/function_registry.h>

namespace rosetta {

    inline void bindFunctions(JsGenerator& generator)
    {
        auto& registry = FunctionRegistry::instance();

        for (const auto& func_name : registry.getFunctionNames()) {
            const auto* func_info = registry.getFunction(func_name);
            if (!func_info)
                continue;

            generator.exports.Set(func_name,
                Napi::Function::New(
                    generator.env, [func_info](const Napi::CallbackInfo& info) -> Napi::Value {
                        try {
                            if (info.Length() != func_info->parameter_types.size()) {
                                std::string err = "Expected "
                                    + std::to_string(func_info->parameter_types.size())
                                    + " arguments, got " + std::to_string(info.Length());
                                Napi::TypeError::New(info.Env(), err).ThrowAsJavaScriptException();
                                return info.Env().Undefined();
                            }

                            // Convert JS arguments to C++ std::any
                            std::vector<std::any> args;
                            for (size_t i = 0; i < info.Length(); ++i) {
                                args.push_back(TypeConverterRegistry::instance().convert_to_cpp(
                                    info[i], func_info->parameter_types[i]));
                            }

                            // Call the function
                            auto result = func_info->invoker(args);

                            // Convert result back to JS
                            return TypeConverterRegistry::instance().convert_to_js(
                                info.Env(), result, func_info->return_type);

                        } catch (const std::exception& e) {
                            Napi::Error::New(info.Env(), e.what()).ThrowAsJavaScriptException();
                            return info.Env().Undefined();
                        }
                    }));
        }
    }

} // namespace rosetta
