/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "lua_generator.h"
#include <rosetta/function_registry.h>

namespace rosetta {

    inline void bindFunctions(sol::state& lua)
    {
        auto& registry = FunctionRegistry::instance();

        for (const auto& func_name : registry.getFunctionNames()) {
            const auto* func_info = registry.getFunction(func_name);
            if (!func_info)
                continue;

            lua.set_function(func_name, [func_info](sol::variadic_args va) -> sol::object {
                if (va.size() != func_info->parameter_types.size()) {
                    throw std::runtime_error("Wrong number of arguments");
                }

                // Convert Lua arguments to C++ std::any
                std::vector<std::any> cpp_args;
                for (size_t i = 0; i < va.size(); ++i) {
                    sol::object arg = va[i];
                    const auto& param_type = func_info->parameter_types[i];

                    if (param_type == "string") {
                        cpp_args.push_back(arg.as<std::string>());
                    } else if (param_type == "int") {
                        cpp_args.push_back(arg.as<int>());
                    } else if (param_type == "double") {
                        cpp_args.push_back(arg.as<double>());
                    }
                    // ... handle other types
                }

                // Call the function
                auto result = func_info->invoker(cpp_args);

                // Convert result back to Lua
                if (!result.has_value() || func_info->return_type == "void") {
                    return sol::lua_nil;
                }

                sol::state_view lua_view = va.lua_state();
                if (func_info->return_type == "string") {
                    return sol::make_object(lua_view, std::any_cast<std::string>(result));
                } else if (func_info->return_type == "int") {
                    return sol::make_object(lua_view, std::any_cast<int>(result));
                } else if (func_info->return_type == "double") {
                    return sol::make_object(lua_view, std::any_cast<double>(result));
                }

                return sol::lua_nil;
            });
        }
    }

} // namespace rosetta