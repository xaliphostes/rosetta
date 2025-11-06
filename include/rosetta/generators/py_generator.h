// ============================================================================
// Python binding generator using Rosetta introspection and pybind11
// No inheritance or wrapping required - pure non-intrusive approach
// ============================================================================
#pragma once

#include <functional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rosetta/rosetta.h>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace py = pybind11;

namespace rosetta::bindings {

    /**
     * @brief Main generator class for Python bindings.
     */
    class PyGenerator {
    public:
        explicit PyGenerator(py::module_ &m);

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param py_name Optional Python name (uses C++ name if empty)
         */
        template <typename T> PyGenerator &bind_class(const std::string &py_name = "");

        /**
         * @brief Bind a free function to Python
         * @tparam Ret Return type
         * @tparam Args Argument types
         * @param name Function name in Python
         * @param func Function pointer
         * @param docstring Optional documentation string
         * @return Reference to this for chaining
         */
        template <typename Ret, typename... Args>
        PyGenerator &bind_function(const std::string &name, Ret (*func)(Args...),
                                   const std::string &docstring = "");

        /**
         * @brief Add utility functions to the module
         */
        PyGenerator &add_utilities();

        /**
         * @brief Add module-level documentation
         */
        PyGenerator &set_doc(const std::string &doc);

    private:
        py::module_ &module_;
    };

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> void bind_classes(PyGenerator &gen);

    // ============================================================================
    // Convenience function for module initialization
    // ============================================================================

    /**
     * @brief Create a Python binding generator for a module
     * @param m The pybind11 module
     * @return A binding generator ready to use
     */
    PyGenerator create_bindings(py::module_ &m);

} // namespace rosetta::bindings

// ============================================================================
// Helper Macros
// ============================================================================

/**
 * @brief Begin pybind11 module with a generator instance
 * Usage:
 *   BEGIN_PY_MODULE(mymodule, "Module documentation") {
 *       gen.bind_classes<Person, Vehicle>();
 *   }
 *   END_PY_MODULE();
 */
#define BEGIN_PY_MODULE(module_name, doc_string) \
    PYBIND11_MODULE(module_name, m) {            \
        m.doc()        = doc_string;             \
        auto generator = rosetta::bindings::create_bindings(m);

/**
 * @brief Bind a class to Python (simplified macro)
 */
#define BIND_PY_CLASS(Class) generator.bind_class<Class>(#Class);

/**
 * @brief Auto-bind a function
 */
#define BIND_FUNCTION(func, doc) generator.bind_function(#func, func, doc)

#define BIND_CONSTANT(name, value) m.attr(name) = value;


#define BIND_PY_UTILITIES() generator.add_utilities();

/**
 * Terminate the pybind11 module
 */
#define END_PY_MODULE() }

#include "inline/py_generator.hxx"