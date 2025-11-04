// ============================================================================
// Automatic pybind11/Python binding generator for Rosetta introspection
// Non-intrusive design - uses Registry and ClassMetadata at runtime
// ============================================================================
#pragma once

#include <rosetta/core/any.h>  // for core::Any
#include <functional>
#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../type_info.h"

// Forward declarations for Rosetta types
namespace rosetta::core {
    class Any;
    class Registry;
    template <typename T> class ClassMetadata;
} // namespace rosetta::core

namespace py = pybind11;

namespace rosetta::generators::python {

    // Type converters between C++ and Python
    using CppToPyConverter = std::function<py::object(const core::Any &)>;
    using PyToCppConverter = std::function<core::Any(const py::object &)>;

    // Class extractor function type
    using ClassExtractor = std::function<bool(const py::object &, core::Any &)>;

    /**
     * @brief Registry for class extractors (singleton)
     * Maps class names to extraction functions
     */
    class ClassExtractorRegistry {
        std::unordered_map<std::string, ClassExtractor> extractors_;

        ClassExtractorRegistry() = default;

    public:
        static ClassExtractorRegistry &instance() {
            static ClassExtractorRegistry registry;
            return registry;
        }

        void register_extractor(const std::string &class_name, ClassExtractor extractor) {
            extractors_[class_name] = std::move(extractor);
        }

        bool try_extract(const std::string &class_name, const py::object &obj,
                         core::Any &out) const {
            auto it = extractors_.find(class_name);
            if (it != extractors_.end()) {
                return it->second(obj, out);
            }
            return false;
        }

        bool try_extract_any(const py::object &obj, core::Any &out) const {
            // Try to extract the class name from Python object
            if (py::hasattr(obj, "__class__")) {
                try {
                    py::object  py_class_name = obj.attr("__class__").attr("__name__");
                    std::string class_name    = py_class_name.cast<std::string>();
                    return try_extract(class_name, obj, out);
                } catch (...) {
                }
            }
            return false;
        }
    };

    /**
     * @brief Register a class extractor for a specific type
     */
    template <typename T> inline void register_class_extractor(const std::string &class_name) {
        ClassExtractorRegistry::instance().register_extractor(
            class_name, [](const py::object &obj, core::Any &out) -> bool {
                try {
                    T &cpp_obj = obj.cast<T &>();
                    out        = core::Any(cpp_obj);
                    return true;
                } catch (...) {
                    return false;
                }
            });
    }

    /**
     * @brief Automatic pybind11/Python binding generator
     *
     * Generates Python bindings at runtime using Rosetta's introspection.
     * No code generation, no inheritance, completely non-intrusive.
     *
     * @example
     * ```cpp
     * PYBIND11_MODULE(mymodule, m) {
     *     PyGenerator generator(m);
     *     generator.bind_classes<Person, Vehicle, Animal>();
     * }
     * ```
     */
    class PyGenerator {
    public:
        /**
         * @brief Constructor
         * @param module pybind11 module
         */
        explicit PyGenerator(py::module_ &module);

        /**
         * @brief Bind a single class to Python
         * @tparam Class C++ class type
         * @param custom_name Custom Python name (optional)
         * @return Reference to this for chaining
         */
        template <typename Class> PyGenerator &bind_class(const std::string &custom_name = "");

        /**
         * @brief Bind multiple classes to Python
         * @tparam Classes C++ class types
         * @return Reference to this for chaining
         */
        template <typename... Classes> PyGenerator &bind_classes();

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
         * @brief Register custom type converter using TypeInfo
         * @tparam T C++ type
         * @param to_py Converter from C++ to Python
         * @param from_py Converter from Python to C++
         * @return Reference to this for chaining
         */
        template <typename T>
        PyGenerator &register_converter(CppToPyConverter to_py, PyToCppConverter from_py);

        /**
         * @brief Register custom type converter by type_index
         * @param type_idx Type index
         * @param to_py Converter from C++ to Python
         * @param from_py Converter from Python to C++
         * @return Reference to this for chaining
         */
        PyGenerator &register_converter(std::type_index type_idx, CppToPyConverter to_py,
                                        PyToCppConverter from_py);

        /**
         * @brief Add utility functions to module
         * @return Reference to this for chaining
         */
        PyGenerator &add_utilities();

        /**
         * @brief Get TypeInfo for a registered type
         */
        template <typename T> const TypeInfo &get_type_info() const;

        /**
         * @brief Set module docstring
         */
        PyGenerator &set_module_doc(const std::string &doc);

        // Public for convenience
        py::module_ &module;

        // TODO: put in private
        // Helper: Get converter for a type
        CppToPyConverter get_to_py_converter(std::type_index idx) const;
        PyToCppConverter get_from_py_converter(std::type_index idx) const;

    private:
        // Track bound classes to avoid duplicates
        std::unordered_set<std::string> bound_classes_;

        // Type converters registry
        std::unordered_map<std::type_index, CppToPyConverter> cpp_to_py_;
        std::unordered_map<std::type_index, PyToCppConverter> py_to_cpp_;

        // Helper: Convert C++ Any to py::object using TypeInfo
        py::object any_to_py(const core::Any &value, const TypeInfo *type_info = nullptr);

        // Helper: Convert py::object to C++ Any using TypeInfo
        core::Any py_to_any(const py::object &value, const TypeInfo *type_info = nullptr);

        // Helper: Create pybind11 class binding for a C++ type
        template <typename Class> void create_class_binding(const std::string &class_name);

        // Helper: Bind fields to Python class
        template <typename Class>
        void bind_fields(py::class_<Class> &py_class, const core::ClassMetadata<Class> &meta);

        // Helper: Bind methods to Python class
        template <typename Class>
        void bind_methods(py::class_<Class> &py_class, const core::ClassMetadata<Class> &meta);

        // Helper: Bind constructor to Python class
        template <typename Class> void bind_constructor(py::class_<Class> &py_class);

        // Initialize default converters
        void init_default_converters();
    };

    // ========================================================================
    // HELPER FUNCTIONS (can be used standalone)
    // ========================================================================

    /**
     * @brief Standalone helper to bind a class
     * @tparam T Class type
     * @param generator Generator instance
     * @param class_name Python name
     */
    template <typename T>
    void bind_class(PyGenerator &generator, const std::string &class_name = "");

    /**
     * @brief Standalone helper to bind multiple classes
     * @tparam Classes Class types
     * @param generator Generator instance
     */
    template <typename... Classes> void bind_classes(PyGenerator &generator);

} // namespace rosetta::generators::python

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

/**
 * @brief Begin pybind11 module with a generator instance
 * Usage:
 *   BEGIN_PY_MODULE(mymodule, "Module documentation") {
 *       gen.bind_classes<Person, Vehicle>();
 *   }
 *   END_PY_MODULE();
 */
#define BEGIN_MODULE(module_name, doc_string) \
    PYBIND11_MODULE(module_name, m) {         \
        m.doc() = doc_string;                 \
        rosetta::generators::python::PyGenerator gen(m);

#define END_MODULE() }

/**
 * @brief Register common type converters
 */
#define REGISTER_COMMON_CONVERTERS() rosetta::generators::python::register_common_converters(gen);

/**
 *
 */
#define REGISTER_UTILITIES() gen.add_utilities()

/**
 * @brief Auto-bind a single class
 */
#define BIND_CLASS(Class) gen.bind_class<Class>(#Class)

/**
 * @brief Auto-bind multiple classes
 */
#define BIND_CLASSES(...) gen.bind_classes<__VA_ARGS__>()

/**
 * @brief Auto-bind a function
 */
#define BIND_FUNCTION(func, doc) gen.bind_function(#func, func, doc)

// Include implementation
#include "inline/py_generator.hxx"