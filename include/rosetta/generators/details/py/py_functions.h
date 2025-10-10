/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "py_generator.h"
#include <rosetta/function_registry.h>

namespace rosetta {

    class PyGenerator;

    /**
     * @brief Helper function to convert Python object to std::any
     */
    inline std::any convert_python_to_any(py::object py_value, const std::string &type_name) {
        if (type_name == "string") {
            return std::make_any<std::string>(py_value.cast<std::string>());
        } else if (type_name == "int") {
            return std::make_any<int>(py_value.cast<int>());
        } else if (type_name == "double") {
            return std::make_any<double>(py_value.cast<double>());
        } else if (type_name == "float") {
            return std::make_any<float>(py_value.cast<float>());
        } else if (type_name == "bool") {
            return std::make_any<bool>(py_value.cast<bool>());
        } else if (type_name == "vector<int>") {
            return std::make_any<std::vector<int>>(py_value.cast<std::vector<int>>());
        } else if (type_name == "vector<double>") {
            return std::make_any<std::vector<double>>(py_value.cast<std::vector<double>>());
        } else if (type_name == "vector<string>") {
            return std::make_any<std::vector<std::string>>(
                py_value.cast<std::vector<std::string>>());
        }

        throw py::type_error("Unsupported type conversion for: " + type_name);
    }

    /**
     * @brief Helper function to convert std::any to Python object
     */
    inline py::object convert_any_to_python(const std::any &value, const std::string &type_name) {
        if (!value.has_value() || type_name == "void") {
            return py::none();
        }

        if (type_name == "string") {
            return py::cast(std::any_cast<std::string>(value));
        } else if (type_name == "int") {
            return py::cast(std::any_cast<int>(value));
        } else if (type_name == "double") {
            return py::cast(std::any_cast<double>(value));
        } else if (type_name == "float") {
            return py::cast(std::any_cast<float>(value));
        } else if (type_name == "bool") {
            return py::cast(std::any_cast<bool>(value));
        } else if (type_name == "vector<int>") {
            return py::cast(std::any_cast<std::vector<int>>(value));
        } else if (type_name == "vector<double>") {
            return py::cast(std::any_cast<std::vector<double>>(value));
        } else if (type_name == "vector<string>") {
            return py::cast(std::any_cast<std::vector<std::string>>(value));
        }

        return py::none();
    }


    /**
     * @brief Bind a single function by name
     */
    inline void bindFunction(PyGenerator &generator, const std::string &func_name) {
        auto       &registry  = FunctionRegistry::instance();
        const auto *func_info = registry.getFunction(func_name);

        if (!func_info) {
            throw std::runtime_error("Function not found: " + func_name);
        }

        generator.module.def(
            func_name.c_str(),
            [func_info](py::args args) -> py::object {
                if (args.size() != func_info->parameter_types.size()) {
                    throw py::value_error("Expected " +
                                          std::to_string(func_info->parameter_types.size()) +
                                          " arguments, got " + std::to_string(args.size()));
                }

                // Convert Python arguments to C++ std::any
                std::vector<std::any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    cpp_args.push_back(
                        convert_python_to_any(args[i], func_info->parameter_types[i]));
                }

                // Call the function
                auto result = func_info->invoker(cpp_args);

                // Convert result back to Python
                return convert_any_to_python(result, func_info->return_type);
            },
            func_name.c_str());
    }

    /**
     * @brief Bind multiple specific functions by name
     */
    inline void bindFunctions(PyGenerator &generator, const std::vector<std::string> &func_names) {
        for (const auto &name : func_names) {
            bindFunction(generator, name);
        }
    }

    /**
     * @brief Bind ALL registered functions
     */
    inline void bindAllFunctions(PyGenerator &generator) {
        auto &registry = FunctionRegistry::instance();

        for (const auto &func_name : registry.getFunctionNames()) {
            bindFunction(generator, func_name);
        }
    }

    // inline void bindFunctions(py::module_ &m) {
    //     auto &registry = FunctionRegistry::instance();

    //     for (const auto &func_name : registry.getFunctionNames()) {
    //         const auto *func_info = registry.getFunction(func_name);
    //         if (!func_info)
    //             continue;

    //         m.def(
    //             func_name.c_str(),
    //             [func_info](py::args args) -> py::object {
    //                 if (args.size() != func_info->parameter_types.size()) {
    //                     throw py::value_error("Wrong number of arguments");
    //                 }

    //                 // Convert Python arguments to C++ std::any
    //                 std::vector<std::any> cpp_args;
    //                 for (size_t i = 0; i < args.size(); ++i) {
    //                     // You'll need to implement convert_python_to_any
    //                     // Similar to what's in PyGenerator
    //                     if (func_info->parameter_types[i] == "string") {
    //                         cpp_args.push_back(args[i].cast<std::string>());
    //                     } else if (func_info->parameter_types[i] == "int") {
    //                         cpp_args.push_back(args[i].cast<int>());
    //                     } else if (func_info->parameter_types[i] == "double") {
    //                         cpp_args.push_back(args[i].cast<double>());
    //                     }
    //                     // ... handle other types
    //                 }

    //                 // Call the function
    //                 auto result = func_info->invoker(cpp_args);

    //                 // Convert result back to Python
    //                 if (!result.has_value() || func_info->return_type == "void") {
    //                     return py::none();
    //                 }

    //                 if (func_info->return_type == "string") {
    //                     return py::cast(std::any_cast<std::string>(result));
    //                 } else if (func_info->return_type == "int") {
    //                     return py::cast(std::any_cast<int>(result));
    //                 } else if (func_info->return_type == "double") {
    //                     return py::cast(std::any_cast<double>(result));
    //                 }

    //                 return py::none();
    //             },
    //             func_name.c_str());
    //     }
    // }

} // namespace rosetta