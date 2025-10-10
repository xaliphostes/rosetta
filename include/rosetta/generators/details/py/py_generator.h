/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <memory>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rosetta/introspectable.h>
#include <typeinfo>
#include <unordered_set>

namespace py = pybind11;

namespace rosetta {

    /**
     * @brief Automatic pybind11 binding generator for introspectable classes
     * @example
     * ```c++
     * // Usage example:
     * PYBIND11_MODULE(my_module, m) {
     *     PythonBindingGenerator generator(m);
     *
     *     // Automatically bind introspectable classes
     *     generator.bind_class<Person>();
     *     generator.bind_class<GameObject>("GameObj");  // Custom name
     *
     *     // Or bind multiple at once
     *     generator.bind_classes<Person, GameObject, Vehicle>();
     *
     *     // Or use the macro
     *     PYBIND11_AUTO_BIND_CLASS(m, Person);
     * }
     * ```
     */
    class PyGenerator {
    public:
        py::module_ &module;

        explicit PyGenerator(py::module_ &m) : module(m) {}

        /**
         * @brief Automatically bind an introspectable class to Python
         * @tparam T The introspectable class type
         * @param class_name Optional custom class name (uses introspection name if
         * empty)
         * @return pybind11 class binding for further customization
         */
        template <typename T> auto bind_class(const std::string &class_name = "") -> py::class_<T>;

        /**
         * @brief Bind multiple classes at once
         */
        template <typename... Classes> void bind_classes();

    private:
        std::unordered_set<std::string> bound_classes;

        template <typename T>
        void bind_constructors(py::class_<T> &py_class, const rosetta::TypeInfo &type_info);

        template <typename T>
        void bind_members(py::class_<T> &py_class, const rosetta::TypeInfo &type_info);

        template <typename T>
        void bind_methods(py::class_<T> &py_class, const rosetta::TypeInfo &type_info);

        template <typename T> void bind_introspection_utilities(py::class_<T> &py_class);

        // Helper function to check if a method is a getter/setter
        bool is_getter_setter_method(const std::string &) const;

        // Convert std::any to Python object based on type name
        py::object convert_any_to_python(const std::any &, const std::string &) const;

        // Convert Python object to std::any based on expected type
        std::any convert_python_to_any(py::object, const std::string &) const;
    };

} // namespace rosetta

// Convenience macro for auto-binding
#define PYBIND11_AUTO_BIND_CLASS(module, ClassName) \
    PyGenerator(module).bind_class<ClassName>(#ClassName)

#include "inline/py_generator.hxx"
