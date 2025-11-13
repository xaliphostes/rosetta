// ============================================================================
// Python binding generator using Rosetta introspection and pybind11
// No inheritance or wrapping required - pure non-intrusive approach
// ============================================================================
#pragma once

#include <array>
#include <deque>
#include <functional>
#include <map>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rosetta/rosetta.h>
#include <set>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace pyb11 = pybind11;

namespace rosetta::py {

    /**
     * @brief Generator for Python bindings.
     */
    class PyGenerator {
    public:
        explicit PyGenerator(pyb11::module_ &m);

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param py_name Optional Python name (uses C++ name if empty)
         */
        template <typename T> PyGenerator &bind_class(const std::string &py_name = "");

        /**
         * @brief Bind a derived class with its base class
         * @tparam Derived The derived C++ class type
         * @tparam Base The base C++ class type
         * @param py_name Optional Python name (uses C++ name if empty)
         */
        template <typename Derived, typename Base>
        PyGenerator &bind_derived_class(const std::string &py_name = "");

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
        pyb11::module_ &module_;
    };

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> void bind_classes(PyGenerator &gen);

    template <typename T> void                     bind_vector_type();
    template <typename K, typename V> void         bind_map_type();
    template <typename T> void                     bind_set_type();
    template <typename T, size_t N> void           bind_array_type();
    template <typename K, typename V> void         bind_unordered_map_type();
    template <typename T> void                     bind_unordered_set_type();
    template <typename T> void                     bind_deque_type();
    template <typename Ret, typename... Args> void register_function_converter();

    // ============================================================================
    // Convenience function for module initialization
    // ============================================================================

    /**
     * @brief Create a Python binding generator for a module
     * @param m The pybind11 module
     * @return A binding generator ready to use
     */
    PyGenerator create_bindings(pyb11::module_ &m);

} // namespace rosetta::py

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
        auto generator = rosetta::py::create_bindings(m);

/**
 * @brief Bind a class to Python (simplified macro)
 */
#define BIND_PY_CLASS(Class) generator.bind_class<Class>(#Class);

/**
 * @brief Bind a derived class with its base class
 */
#define BIND_PY_DERIVED_CLASS(Derived, Base) generator.bind_derived_class<Derived, Base>(#Derived);

/**
 * @brief Bind multiple classes to Pyhton
 */
#define BIND_PY_CLASSES(...) ([&]() { (generator.bind_class<__VA_ARGS__>(#__VA_ARGS__), ...); })();

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