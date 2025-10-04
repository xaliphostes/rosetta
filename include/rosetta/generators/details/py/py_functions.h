/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "py_generator.h"
#include <rosetta/function_registry.h>

namespace rosetta {

    inline void bindFunctions(py::module_ &m) {
        auto &registry = FunctionRegistry::instance();

        for (const auto &func_name : registry.getFunctionNames()) {
            const auto *func_info = registry.getFunction(func_name);
            if (!func_info)
                continue;

            m.def(
                func_name.c_str(),
                [func_info](py::args args) -> py::object {
                    if (args.size() != func_info->parameter_types.size()) {
                        throw py::value_error("Wrong number of arguments");
                    }

                    // Convert Python arguments to C++ std::any
                    std::vector<std::any> cpp_args;
                    for (size_t i = 0; i < args.size(); ++i) {
                        // You'll need to implement convert_python_to_any
                        // Similar to what's in PyGenerator
                        if (func_info->parameter_types[i] == "string") {
                            cpp_args.push_back(args[i].cast<std::string>());
                        } else if (func_info->parameter_types[i] == "int") {
                            cpp_args.push_back(args[i].cast<int>());
                        } else if (func_info->parameter_types[i] == "double") {
                            cpp_args.push_back(args[i].cast<double>());
                        }
                        // ... handle other types
                    }

                    // Call the function
                    auto result = func_info->invoker(cpp_args);

                    // Convert result back to Python
                    if (!result.has_value() || func_info->return_type == "void") {
                        return py::none();
                    }

                    if (func_info->return_type == "string") {
                        return py::cast(std::any_cast<std::string>(result));
                    } else if (func_info->return_type == "int") {
                        return py::cast(std::any_cast<int>(result));
                    } else if (func_info->return_type == "double") {
                        return py::cast(std::any_cast<double>(result));
                    }

                    return py::none();
                },
                func_name.c_str());
        }
    }

} // namespace rosetta