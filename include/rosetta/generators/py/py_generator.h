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
#include <vector>

namespace py = pybind11;

namespace rosetta::bindings {

    // ============================================================================
    // Type Converters - Convert between C++ Any and Python objects
    // ============================================================================

    /**
     * @brief Convert C++ Any to Python object
     */
    inline py::object any_to_python(const core::Any &value) {
        if (!value.has_value()) {
            return py::none();
        }

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) {
            return py::cast(value.as<int>());
        }
        if (type == std::type_index(typeid(double))) {
            return py::cast(value.as<double>());
        }
        if (type == std::type_index(typeid(float))) {
            return py::cast(value.as<float>());
        }
        if (type == std::type_index(typeid(bool))) {
            return py::cast(value.as<bool>());
        }
        if (type == std::type_index(typeid(std::string))) {
            return py::cast(value.as<std::string>());
        }
        if (type == std::type_index(typeid(size_t))) {
            return py::cast(value.as<size_t>());
        }
        if (type == std::type_index(typeid(long))) {
            return py::cast(value.as<long>());
        }
        if (type == std::type_index(typeid(long long))) {
            return py::cast(value.as<long long>());
        }

        // Void return (method that returns nothing)
        if (type == std::type_index(typeid(void))) {
            return py::none();
        }

        // For complex types, return None or handle specially
        return py::none();
    }

    /**
     * @brief Convert Python object to C++ Any
     */
    inline core::Any python_to_any(const py::object &py_obj, std::type_index expected_type) {
        // Handle None
        if (py_obj.is_none()) {
            return core::Any();
        }

        // Handle primitives
        if (expected_type == std::type_index(typeid(int))) {
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(py_obj.cast<int>());
            }
        }
        if (expected_type == std::type_index(typeid(double))) {
            if (py::isinstance<py::float_>(py_obj)) {
                return core::Any(py_obj.cast<double>());
            }
            // Python int can convert to double
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(static_cast<double>(py_obj.cast<int>()));
            }
        }
        if (expected_type == std::type_index(typeid(float))) {
            if (py::isinstance<py::float_>(py_obj)) {
                return core::Any(static_cast<float>(py_obj.cast<double>()));
            }
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(static_cast<float>(py_obj.cast<int>()));
            }
        }
        if (expected_type == std::type_index(typeid(bool))) {
            if (py::isinstance<py::bool_>(py_obj)) {
                return core::Any(py_obj.cast<bool>());
            }
        }
        if (expected_type == std::type_index(typeid(std::string))) {
            if (py::isinstance<py::str>(py_obj)) {
                return core::Any(py_obj.cast<std::string>());
            }
        }
        if (expected_type == std::type_index(typeid(size_t))) {
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(py_obj.cast<size_t>());
            }
        }
        if (expected_type == std::type_index(typeid(long))) {
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(py_obj.cast<long>());
            }
        }
        if (expected_type == std::type_index(typeid(long long))) {
            if (py::isinstance<py::int_>(py_obj)) {
                return core::Any(py_obj.cast<long long>());
            }
        }

        return core::Any(); // Empty
    }

    // ============================================================================
    // Python Class Binding Helper
    // ============================================================================

    /**
     * @brief Helper to bind a single class using Rosetta metadata
     */
    template <typename T> class PyClassBinder {
    public:
        /**
         * @brief Bind the class to Python module
         */
        static void bind(py::module_ &m, const std::string &py_name = "") {
            // Get metadata from registry
            const auto &meta       = core::Registry::instance().get<T>();
            std::string final_name = py_name.empty() ? meta.name() : py_name;

            // Create the pybind11 class
            py::class_<T> py_class(m, final_name.c_str());

            // Bind constructors
            bind_constructors(py_class, meta);

            // Bind fields
            bind_fields(py_class, meta);

            // Bind methods
            bind_methods(py_class, meta);

            // Add __repr__ for better debugging
            py_class.def("__repr__",
                         [final_name](const T &obj) { return "<" + final_name + " object>"; });
        }

    private:
        /**
         * @brief Bind all constructors
         */
        static void bind_constructors(py::class_<T> &py_class, const core::ClassMetadata<T> &meta) {
            const auto &ctors = meta.constructors();

            if (ctors.empty()) {
                // No registered constructors, try default
                if constexpr (std::is_default_constructible_v<T>) {
                    py_class.def(py::init<>());
                }
                return;
            }

            // Try to detect constructor signatures by calling with different arg counts
            for (size_t ctor_idx = 0; ctor_idx < ctors.size(); ++ctor_idx) {
                const auto &ctor = ctors[ctor_idx];

                // Detect arity (number of parameters)
                int arity = detect_constructor_arity(ctor);

                if (arity == 0) {
                    // Default constructor
                    py_class.def(py::init([ctor]() {
                        std::vector<core::Any> args;
                        core::Any              result = ctor(args);
                        return result.as<T>();
                    }));
                } else if (arity > 0) {
                    // Parametric constructor - we'll create a generic version
                    // that accepts Python objects and tries to convert them
                    bind_parametric_constructor(py_class, ctor, arity);
                }
            }
        }

        /**
         * @brief Detect constructor arity
         */
        static int
        detect_constructor_arity(const std::function<core::Any(std::vector<core::Any>)> &ctor) {

            for (int k = 0; k <= 10; ++k) {
                std::vector<core::Any> args;
                args.reserve(k);
                for (int i = 0; i < k; ++i) {
                    args.emplace_back(0); // Placeholder
                }

                try {
                    (void)ctor(args);
                    return k;
                } catch (const std::runtime_error &e) {
                    std::string msg(e.what());
                    if (msg.find("Constructor argument count mismatch") != std::string::npos) {
                        continue; // Try next k
                    }
                    return k; // Type mismatch => count matched
                } catch (...) {
                    return k; // Conservative: count matched
                }
            }
            return -1;
        }

        /**
         * @brief Bind a parametric constructor with generic Python arguments
         */
        static void
        bind_parametric_constructor(py::class_<T> &py_class,
                                    const std::function<core::Any(std::vector<core::Any>)> &ctor,
                                    int                                                     arity) {

            // Create a lambda that accepts py::args
            py_class.def(py::init([ctor, arity](py::args args) {
                if (static_cast<int>(args.size()) != arity) {
                    throw std::runtime_error("Constructor expects " + std::to_string(arity) +
                                             " arguments, got " + std::to_string(args.size()));
                }

                // Convert Python args to Any - we'll try some common types
                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    py::object arg = args[i];

                    // Try different types in order of likelihood
                    if (py::isinstance<py::int_>(arg)) {
                        cpp_args.emplace_back(arg.cast<int>());
                    } else if (py::isinstance<py::float_>(arg)) {
                        cpp_args.emplace_back(arg.cast<double>());
                    } else if (py::isinstance<py::bool_>(arg)) {
                        cpp_args.emplace_back(arg.cast<bool>());
                    } else if (py::isinstance<py::str>(arg)) {
                        cpp_args.emplace_back(arg.cast<std::string>());
                    } else {
                        throw std::runtime_error("Unsupported argument type");
                    }
                }

                core::Any result = ctor(cpp_args);
                return result.as<T>();
            }));
        }

        /**
         * @brief Bind all fields as properties
         */
        static void bind_fields(py::class_<T> &py_class, const core::ClassMetadata<T> &meta) {
            const auto &fields = meta.fields();

            for (const auto &field_name : fields) {
                std::type_index field_type = meta.get_field_type(field_name);

                // Create getter
                auto getter = [field_name, &meta](T &obj) -> py::object {
                    try {
                        core::Any value = meta.get_field(obj, field_name);
                        return any_to_python(value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error(std::string("Failed to get field '") + field_name +
                                                 "': " + e.what());
                    }
                };

                // Create setter
                auto setter = [field_name, field_type, &meta](T &obj, py::object value) {
                    try {
                        core::Any cpp_value = python_to_any(value, field_type);
                        meta.set_field(obj, field_name, cpp_value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error(std::string("Failed to set field '") + field_name +
                                                 "': " + e.what());
                    }
                };

                // Register property with both getter and setter
                py_class.def_property(field_name.c_str(), getter, setter);
            }
        }

        /**
         * @brief Bind all methods
         */
        static void bind_methods(py::class_<T> &py_class, const core::ClassMetadata<T> &meta) {
            const auto &methods = meta.methods();

            for (const auto &method_name : methods) {
                size_t          arity       = meta.get_method_arity(method_name);
                const auto     &arg_types   = meta.get_method_arg_types(method_name);
                std::type_index return_type = meta.get_method_return_type(method_name);

                // Create method wrapper that accepts py::args
                auto method_wrapper = [method_name, arity, arg_types, return_type,
                                       &meta](T &obj, py::args args) -> py::object {
                    if (args.size() != arity) {
                        throw std::runtime_error("Method '" + method_name + "' expects " +
                                                 std::to_string(arity) + " arguments, got " +
                                                 std::to_string(args.size()));
                    }

                    // Convert Python arguments to C++ Any
                    std::vector<core::Any> cpp_args;
                    for (size_t i = 0; i < args.size(); ++i) {
                        cpp_args.push_back(python_to_any(args[i], arg_types[i]));
                    }

                    // Invoke the method
                    try {
                        core::Any result = meta.invoke_method(obj, method_name, cpp_args);
                        return any_to_python(result);
                    } catch (const std::exception &e) {
                        throw std::runtime_error(std::string("Method '") + method_name +
                                                 "' failed: " + e.what());
                    }
                };

                // Register the method
                py_class.def(method_name.c_str(), method_wrapper);
            }
        }
    };

    // ============================================================================
    // Python Binding Generator
    // ============================================================================

    /**
     * @brief Main generator class for Python bindings
     */
    class PyBindingGenerator {
    public:
        explicit PyBindingGenerator(py::module_ &m) : module_(m) {}

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param py_name Optional Python name (uses C++ name if empty)
         */
        template <typename T> PyBindingGenerator &bind_class(const std::string &py_name = "") {
            PyClassBinder<T>::bind(module_, py_name);
            return *this;
        }

        /**
         * @brief Add utility functions to the module
         */
        PyBindingGenerator &add_utilities() {
            // List all registered classes
            module_.def(
                "list_classes", []() { return core::Registry::instance().list_classes(); },
                "List all registered classes");

            // Get class information
            module_.def(
                "get_class_info",
                [](const std::string &name) -> py::dict {
                    auto holder = core::Registry::instance().get_by_name(name);
                    if (!holder) {
                        throw std::runtime_error("Class not found: " + name);
                    }

                    py::dict info;
                    info["name"] = holder->get_name();

                    const auto &inh                = holder->get_inheritance();
                    info["is_abstract"]            = inh.is_abstract;
                    info["is_polymorphic"]         = inh.is_polymorphic;
                    info["has_virtual_destructor"] = inh.has_virtual_destructor;
                    info["base_count"]             = inh.total_base_count();

                    return info;
                },
                py::arg("name"), "Get information about a registered class");

            // Version info
            module_.def("version", []() { return rosetta::version(); }, "Get Rosetta version");

            return *this;
        }

        /**
         * @brief Add module-level documentation
         */
        PyBindingGenerator &set_doc(const std::string &doc) {
            module_.doc() = doc.c_str();
            return *this;
        }

    private:
        py::module_ &module_;
    };

// ============================================================================
// Helper Macros
// ============================================================================

/**
 * @brief Bind a class to Python (simplified macro)
 */
#define ROSETTA_BIND_PY_CLASS(Generator, ClassName) Generator.bind_class<ClassName>(#ClassName)

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> void bind_classes(PyBindingGenerator &gen) {
        (gen.bind_class<Classes>(), ...);
    }

    // ============================================================================
    // Convenience function for module initialization
    // ============================================================================

    /**
     * @brief Create a Python binding generator for a module
     * @param m The pybind11 module
     * @return A binding generator ready to use
     */
    inline PyBindingGenerator create_bindings(py::module_ &m) {
        return PyBindingGenerator(m);
    }

} // namespace rosetta::bindings