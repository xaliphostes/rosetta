// ============================================================================
// rosetta/generators/python_binder.h
//
// Unified Python binding system using Rosetta introspection and pybind11
// Combines manual and automatic binding approaches
// ============================================================================
#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rosetta/core/any.h>
#include <rosetta/core/class_metadata.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace py = pybind11;

namespace rosetta::python {

    // ============================================================================
    // Forward declarations
    // ============================================================================

    class BinderRegistry;
    class PythonBinder;

    // ============================================================================
    // Type Conversion System (shared by all binding methods)
    // ============================================================================

    /**
     * @brief Converts between C++ (via Any) and Python objects
     *
     * This is the core conversion layer used by both manual and automatic binding
     */
    class TypeConverter {
    public:
        /**
         * @brief Convert C++ Any to Python object
         */
        static py::object any_to_python(const core::Any &value);

        /**
         * @brief Convert Python object to C++ Any with type hint
         */
        static core::Any python_to_any(const py::object &obj, std::type_index expected_type);

        /**
         * @brief Convert Python object to C++ Any (auto-detect type)
         */
        static core::Any python_to_any(const py::object &obj);

        /**
         * @brief Register a custom converter for a user type
         */
        template <typename T> static void register_type();

    private:
        struct ConverterRegistry {
            std::unordered_map<std::type_index, std::function<core::Any(const py::object &)>>
                to_cpp;
            std::unordered_map<std::type_index, std::function<py::object(const core::Any &)>>
                to_python;

            static ConverterRegistry &instance();
        };
    };

    // ============================================================================
    // Class Binder (shared implementation for binding classes)
    // ============================================================================

    /**
     * @brief Template class that handles binding of a single C++ class
     *
     * Used internally by both manual and automatic binding systems
     */
    template <typename Class> class ClassBinder {
    public:
        /**
         * @brief Bind a class to a Python module
         * @param m Python module
         * @param name Class name (uses C++ name if empty)
         */
        static py::class_<Class> bind(py::module &m, const std::string &name = "");

    private:
        static void bind_constructors(py::class_<Class>                &py_class,
                                      const core::ClassMetadata<Class> &meta);
        static void bind_fields(py::class_<Class>                &py_class,
                                const core::ClassMetadata<Class> &meta);
        static void bind_methods(py::class_<Class>                &py_class,
                                 const core::ClassMetadata<Class> &meta);
        static void bind_static_methods(py::class_<Class>                &py_class,
                                        const core::ClassMetadata<Class> &meta);
    };

    // ============================================================================
    // Binder Registry (for automatic binding)
    // ============================================================================

    /**
     * @brief Registry for automatic class binding
     *
     * Maintains type-erased binders that can be invoked at module creation time
     */
    class BinderRegistry {
    public:
        static BinderRegistry &instance();

        /**
         * @brief Register a class for automatic binding
         */
        template <typename Class> void register_class(const std::string &name);

        /**
         * @brief Bind all registered classes to a module
         */
        void bind_all(py::module &m, std::function<bool(const std::string &)> filter = nullptr);

        /**
         * @brief Check if a class is registered
         */
        bool has_class(const std::string &name) const;

        /**
         * @brief Clear all registered binders
         */
        void clear();

    private:
        struct TypeErasedBinder {
            virtual ~TypeErasedBinder()      = default;
            virtual void bind(py::module &m) = 0;
        };

        template <typename Class> struct TypedBinder : TypeErasedBinder {
            std::string name;
            explicit TypedBinder(std::string n) : name(std::move(n)) {}
            void bind(py::module &m) override { ClassBinder<Class>::bind(m, name); }
        };

        std::unordered_map<std::string, std::unique_ptr<TypeErasedBinder>> binders_;
    };

    // ============================================================================
    // Python Binder (main API)
    // ============================================================================

    /**
     * @brief Main Python binding interface
     *
     * Provides both manual binding (for fine control) and automatic binding
     * (for convenience). You can mix both approaches.
     */
    class PythonBinder {
    public:
        explicit PythonBinder(py::module &m);

        // ------------------------------------------------------------------------
        // Manual binding (explicit control)
        // ------------------------------------------------------------------------

        /**
         * @brief Manually bind a specific class
         * @tparam Class The C++ class type
         * @param name Python name (uses C++ name if empty)
         * @return Reference to this for chaining
         */
        template <typename Class> PythonBinder &bind_class(const std::string &name = "");

        /**
         * @brief Bind a free function
         * @tparam Ret Return type
         * @tparam Args Argument types
         * @param name Function name in Python
         * @param func Function pointer
         * @param docstring Optional documentation
         * @return Reference to this for chaining
         */
        template <typename Ret, typename... Args>
        PythonBinder &bind_function(const std::string &name, Ret (*func)(Args...),
                                    const std::string &docstring = "");

        /**
         * @brief Bind a constant value
         */
        template <typename T> PythonBinder &bind_constant(const std::string &name, T value);

        // ------------------------------------------------------------------------
        // Automatic binding (convenience)
        // ------------------------------------------------------------------------

        /**
         * @brief Automatically bind all classes registered in BinderRegistry
         * @param filter Optional filter function to select which classes to bind
         * @return Reference to this for chaining
         */
        PythonBinder &
        bind_all_registered(std::function<bool(const std::string &)> filter = nullptr);

        // ------------------------------------------------------------------------
        // Utilities
        // ------------------------------------------------------------------------

        /**
         * @brief Add utility functions to the module (introspection, etc.)
         */
        PythonBinder &add_utilities();

        /**
         * @brief Set module documentation
         */
        PythonBinder &set_doc(const std::string &doc);

        /**
         * @brief Get the underlying pybind11 module
         */
        py::module &module() { return module_; }

    private:
        py::module &module_;
    };

    // ============================================================================
    // Convenience functions
    // ============================================================================

    /**
     * @brief Create a binder for a module
     */
    inline PythonBinder create_binder(py::module &m) {
        return PythonBinder(m);
    }

} // namespace rosetta::python

// ============================================================================
// Simplified Macros (backward compatible with old API)
// ============================================================================

/**
 * @brief Begin module definition with automatic and manual binding support
 *
 * Usage:
 * ```cpp
 * ROSETTA_PY_MODULE(mymodule, "Documentation") {
 *     // Manual binding
 *     binder.bind_class<MyClass>("MyClass");
 *
 *     // Or automatic binding
 *     binder.bind_all_registered();
 * }
 * ROSETTA_PY_MODULE_END
 * ```
 */
#define ROSETTA_PY_MODULE(module_name, doc_string) \
    PYBIND11_MODULE(module_name, m) {              \
        m.doc()     = doc_string;                  \
        auto binder = rosetta::python::create_binder(m);

/**
 * @brief End module definition
 */
#define ROSETTA_PY_MODULE_END() }

/**
 * @brief Register a class with both Rosetta and Python binding registry
 *
 * This replaces ROSETTA_REGISTER_CLASS when you want automatic binding
 */
#define ROSETTA_REGISTER_CLASS_PY(ClassName)                                               \
    []() -> auto & {                                                                       \
        std::cerr << "--> register class " << #ClassName << std::endl;                     \
        auto &meta = rosetta::Registry::instance().register_class<ClassName>(#ClassName);  \
        rosetta::python::BinderRegistry::instance().register_class<ClassName>(#ClassName); \
        rosetta::python::TypeConverter::register_type<ClassName>();                        \
        return meta;                                                                       \
    }()

/**
 * @brief Bind a single class manually (for use inside ROSETTA_PY_MODULE)
 */
#define BIND_CLASS(Class) binder.bind_class<Class>(#Class)

/**
 * @brief Bind all automatically registered classes
 */
#define BIND_ALL_CLASSES() binder.bind_all_registered()

/**
 * @brief Bind a function
 */
#define BIND_FUNCTION(func, doc) binder.bind_function(#func, func, doc)

/**
 * @brief Bind a constant
 */
#define BIND_CONSTANT(name, value) binder.bind_constant(name, value)

/**
 * @brief Add utility functions
 */
#define BIND_UTILITIES() binder.add_utilities()

#include "inline/py_binder.hxx"