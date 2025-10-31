// ============================================================================
// Implementation of PyGenerator - FIXED VERSION for handling C++ objects
// ============================================================================
#pragma once

#include "../../../rosetta.h"
#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <sstream>
#include <stdexcept>

namespace rosetta::generators::python {

    // ========================================================================
    // PYGENERATOR IMPLEMENTATION
    // ========================================================================

    inline PyGenerator::PyGenerator(py::module_ &m) : module(m) { init_default_converters(); }

    inline void PyGenerator::init_default_converters() {
        // Register basic types in TypeRegistry
        TypeRegistry::instance().register_type<int>("int");
        TypeRegistry::instance().register_type<double>("double");
        TypeRegistry::instance().register_type<float>("float");
        TypeRegistry::instance().register_type<bool>("bool");
        TypeRegistry::instance().register_type<std::string>("string");

        // int
        register_converter<int>(
            [](const core::Any &val) { return py::cast(val.as<int>()); },
            [](const py::object &val) { return core::Any(val.cast<int>()); });

        // double
        register_converter<double>(
            [](const core::Any &val) { return py::cast(val.as<double>()); },
            [](const py::object &val) { return core::Any(val.cast<double>()); });

        // float
        register_converter<float>(
            [](const core::Any &val) { return py::cast(val.as<float>()); },
            [](const py::object &val) { return core::Any(val.cast<float>()); });

        // bool
        register_converter<bool>(
            [](const core::Any &val) { return py::cast(val.as<bool>()); },
            [](const py::object &val) { return core::Any(val.cast<bool>()); });

        // std::string
        register_converter<std::string>(
            [](const core::Any &val) { return py::cast(val.as<std::string>()); },
            [](const py::object &val) { return core::Any(val.cast<std::string>()); });
    }

    template <typename T>
    inline PyGenerator &PyGenerator::register_converter(CppToPyConverter to_py,
                                                        PyToCppConverter from_py) {

        // Register type in TypeRegistry if not already registered
        if (!TypeRegistry::instance().has_type<T>()) {
            TypeRegistry::instance().register_type<T>();
        }

        std::type_index idx(typeid(T));

        cpp_to_py_[idx] = std::move(to_py);
        py_to_cpp_[idx] = std::move(from_py);
        return *this;
    }

    inline PyGenerator &PyGenerator::register_converter(std::type_index  type_idx,
                                                        CppToPyConverter to_py,
                                                        PyToCppConverter from_py) {

        cpp_to_py_[type_idx] = std::move(to_py);
        py_to_cpp_[type_idx] = std::move(from_py);
        return *this;
    }

    inline CppToPyConverter PyGenerator::get_to_py_converter(std::type_index idx) const {
        auto it = cpp_to_py_.find(idx);
        if (it != cpp_to_py_.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline PyToCppConverter PyGenerator::get_from_py_converter(std::type_index idx) const {
        auto it = py_to_cpp_.find(idx);
        if (it != py_to_cpp_.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline py::object PyGenerator::any_to_py(const core::Any &value, const TypeInfo *type_info) {
        if (!value.has_value()) {
            return py::none();
        }

        // Get type index from Any
        std::type_index idx = value.get_type_index();

        // Try to find registered converter (preferred method)
        auto converter = get_to_py_converter(idx);
        if (converter) {
            try {
                return converter(value);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Converter failed: " << e.what() << std::endl;
                // Fall through to fallback
            }
        }

        // FALLBACK: Direct type conversion using mangled name
        std::string mangled = value.type_name();

        try {
            // Double
            if (mangled == "d") {
                return py::cast(value.as<double>());
            }

            // Float
            if (mangled == "f") {
                return py::cast(value.as<float>());
            }

            // Int (various representations)
            if (mangled == "i" || mangled == "l" || mangled == "x") {
                return py::cast(value.as<int>());
            }

            // Bool
            if (mangled == "b") {
                return py::cast(value.as<bool>());
            }

            // String (check for various std::string manglings)
            if (mangled.find("string") != std::string::npos ||
                mangled.find("basic_string") != std::string::npos || mangled == "Ss") {
                return py::cast(value.as<std::string>());
            }

            // Vector<double>
            if (mangled.find("vectorIdN") != std::string::npos ||
                mangled.find("6vectorIdN") != std::string::npos) {
                try {
                    const std::vector<double> &vec = value.as<std::vector<double>>();
                    return py::cast(vec);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<double>: " << e.what()
                              << std::endl;
                    return py::list();
                }
            }

            // Vector<int>
            if (mangled.find("vectorIiN") != std::string::npos ||
                mangled.find("6vectorIiN") != std::string::npos) {
                try {
                    const std::vector<int> &vec = value.as<std::vector<int>>();
                    return py::cast(vec);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<int>: " << e.what() << std::endl;
                    return py::list();
                }
            }

            // Vector<float>
            if (mangled.find("vectorIfN") != std::string::npos ||
                mangled.find("6vectorIfN") != std::string::npos) {
                try {
                    const std::vector<float> &vec = value.as<std::vector<float>>();
                    return py::cast(vec);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<float>: " << e.what()
                              << std::endl;
                    return py::list();
                }
            }

            // Vector<string>
            if ((mangled.find("vector") != std::string::npos &&
                 mangled.find("basic_string") != std::string::npos) ||
                mangled.find("6vectorINS_12basic_string") != std::string::npos) {
                try {
                    const std::vector<std::string> &vec = value.as<std::vector<std::string>>();
                    return py::cast(vec);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] Failed to convert vector<string>: " << e.what()
                              << std::endl;
                    return py::list();
                }
            }

            // Optional<double>
            if (mangled.find("optionalIdEE") != std::string::npos ||
                mangled.find("8optionalIdEE") != std::string::npos) {
                std::optional<double> opt = value.as<std::optional<double>>();
                if (opt.has_value()) {
                    return py::cast(*opt);
                }
                return py::none();
            }

            // Optional<int>
            if (mangled.find("optionalIiEE") != std::string::npos ||
                mangled.find("8optionalIiEE") != std::string::npos) {
                std::optional<int> opt = value.as<std::optional<int>>();
                if (opt.has_value()) {
                    return py::cast(*opt);
                }
                return py::none();
            }

            // Optional<float>
            if (mangled.find("optionalIfEE") != std::string::npos ||
                mangled.find("8optionalIfEE") != std::string::npos) {
                std::optional<float> opt = value.as<std::optional<float>>();
                if (opt.has_value()) {
                    return py::cast(*opt);
                }
                return py::none();
            }

            // Optional<bool>
            if (mangled.find("optionalIbEE") != std::string::npos ||
                mangled.find("8optionalIbEE") != std::string::npos) {
                std::optional<bool> opt = value.as<std::optional<bool>>();
                if (opt.has_value()) {
                    return py::cast(*opt);
                }
                return py::none();
            }

            // Optional<string>
            if (mangled.find("optional") != std::string::npos &&
                mangled.find("basic_string") != std::string::npos) {
                std::optional<std::string> opt = value.as<std::optional<std::string>>();
                if (opt.has_value()) {
                    return py::cast(*opt);
                }
                return py::none();
            }

        } catch (const std::bad_cast &e) {
            std::cerr << "[ERROR] Type cast failed for mangled type: " << mangled << std::endl;
            return py::none();
        } catch (const std::exception &e) {
            std::cerr << "[ERROR] Conversion failed: " << e.what() << std::endl;
            return py::none();
        }

        // If we get here, we don't know how to convert this type
        std::cerr << "[ERROR] No converter or fallback for type: " << mangled
                  << " (type_index: " << idx.name() << ")" << std::endl;

        return py::none();
    }

    inline core::Any PyGenerator::py_to_any(const py::object &value, const TypeInfo *type_info) {
        if (value.is_none()) {
            return core::Any();
        }

        // Try to extract as a registered C++ class first using the registry
        core::Any result;
        if (ClassExtractorRegistry::instance().try_extract_any(value, result)) {
            return result;
        }

        // If we have type info, use it to guide conversion
        if (type_info) {
            // Try to find converter by type_index
            auto converter = get_from_py_converter(type_info->type_index);
            if (converter) {
                return converter(value);
            }

            // Fallback based on TypeInfo category
            switch (type_info->category) {
            case TypeCategory::Primitive:
                if (type_info->is_integer() && PyLong_Check(value.ptr())) {
                    return core::Any(value.cast<int>());
                }
                if (type_info->is_floating() && PyFloat_Check(value.ptr())) {
                    return core::Any(value.cast<double>());
                }
                if (type_info->name == "bool" && PyBool_Check(value.ptr())) {
                    return core::Any(value.cast<bool>());
                }
                break;

            case TypeCategory::String:
                if (PyUnicode_Check(value.ptr())) {
                    return core::Any(value.cast<std::string>());
                }
                break;

            default:
                break;
            }
        }

        // Fallback: infer from Python type
        if (PyLong_Check(value.ptr())) {
            return core::Any(value.cast<int>());
        }
        if (PyFloat_Check(value.ptr())) {
            return core::Any(value.cast<double>());
        }
        if (PyBool_Check(value.ptr())) {
            return core::Any(value.cast<bool>());
        }
        if (PyUnicode_Check(value.ptr())) {
            return core::Any(value.cast<std::string>());
        }

        // Handle lists - convert to vector<double> for numeric arrays
        if (PyList_Check(value.ptr())) {
            py::list lst = value.cast<py::list>();

            if (lst.size() == 0) {
                return core::Any(std::vector<double>());
            }

            // Check first element to determine type
            py::object first_elem = lst[0];

            if (PyUnicode_Check(first_elem.ptr())) {
                // String list
                std::vector<std::string> str_vec;
                str_vec.reserve(lst.size());
                for (const auto &item : lst) {
                    str_vec.push_back(item.cast<std::string>());
                }
                return core::Any(std::move(str_vec));
            } else if (PyFloat_Check(first_elem.ptr()) || PyLong_Check(first_elem.ptr())) {
                // Numeric list - use vector<double>
                std::vector<double> vec;
                vec.reserve(lst.size());
                for (const auto &item : lst) {
                    vec.push_back(item.cast<double>());
                }
                return core::Any(std::move(vec));
            }

            // Unknown list type
            return core::Any(std::vector<double>());
        }

        return core::Any();
    }

    template <typename T> inline const TypeInfo &PyGenerator::get_type_info() const {
        return TypeRegistry::instance().get<T>();
    }

    template <typename Class>
    inline void PyGenerator::create_class_binding(const std::string &class_name) {
        // Get metadata from registry
        auto &registry = core::Registry::instance();
        if (!registry.has_class<Class>()) {
            throw std::runtime_error("Class not registered: " + class_name);
        }

        const auto &meta = registry.get<Class>();

        // Create pybind11 class binding
        py::class_<Class> py_class(module, class_name.c_str());

        // Set docstring
        std::string doc = "C++ class: " + class_name;
        py_class.doc() = doc.c_str();

        // Bind constructor
        bind_constructor<Class>(py_class);

        // Bind fields
        bind_fields<Class>(py_class, meta);

        // Bind methods
        bind_methods<Class>(py_class, meta);

        // Add __repr__
        py_class.def("__repr__", [class_name](const Class &) {
            return "<" + class_name + " object>";
        });
    }

    template <typename Class>
    inline void PyGenerator::bind_constructor(py::class_<Class> &py_class) {
        // Only bind default constructor if class is instantiable
        if constexpr (std::is_default_constructible_v<Class>) {
            py_class.def(py::init<>());
        }
    }

    template <typename Class>
    inline void PyGenerator::bind_fields(py::class_<Class>           &py_class,
                                         const core::ClassMetadata<Class> &meta) {
        for (const auto &field_name : meta.fields()) {
            // Capture 'this' pointer to access converters
            PyGenerator *gen_ptr = this;

            // Create getter
            auto getter = [field_name, gen_ptr, &meta](Class &obj) -> py::object {
                core::Any field_value = meta.get_field(obj, field_name);
                return gen_ptr->any_to_py(field_value, nullptr);
            };

            // Create setter
            auto setter = [field_name, gen_ptr, &meta](Class &obj, py::object value) {
                core::Any cpp_value = gen_ptr->py_to_any(value, nullptr);
                try {
                    meta.set_field(obj, field_name, cpp_value);
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Field setter failed: ") + e.what());
                }
            };

            // Bind as property
            py_class.def_property(field_name.c_str(), getter, setter);
        }
    }

    template <typename Class>
    inline void PyGenerator::bind_methods(py::class_<Class>           &py_class,
                                          const core::ClassMetadata<Class> &meta) {
        for (const auto &method_name : meta.methods()) {
            // Capture 'this' pointer to access converters
            PyGenerator *gen_ptr = this;

            auto method_wrapper = [method_name, gen_ptr, &meta](Class &obj, py::args args,
                                                                py::kwargs kwargs) -> py::object {
                // Convert Python arguments to C++ Any
                std::vector<core::Any> cpp_args;
                
                for (size_t i = 0; i < args.size(); ++i) {
                    py::object arg = py::reinterpret_borrow<py::object>(args[i]);
                    core::Any converted = gen_ptr->py_to_any(arg, nullptr);
                    cpp_args.push_back(converted);
                }

                // Invoke method
                try {
                    core::Any result = meta.invoke_method(obj, method_name, cpp_args);
                    return gen_ptr->any_to_py(result, nullptr);
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Method invocation failed: ") + e.what());
                }
            };

            // Bind method
            py_class.def(method_name.c_str(), method_wrapper);
        }
    }

    template <typename Class>
    inline PyGenerator &PyGenerator::bind_class(const std::string &custom_name) {
        auto &registry = core::Registry::instance();

        if (!registry.has_class<Class>()) {
            throw std::runtime_error("Class not registered in Rosetta: " +
                                     std::string(typeid(Class).name()));
        }

        const auto &meta       = registry.get<Class>();
        std::string class_name = custom_name.empty() ? meta.name() : custom_name;

        // Check if already bound
        if (bound_classes_.count(class_name)) {
            return *this;
        }

        // Create class binding
        create_class_binding<Class>(class_name);

        // Mark as bound
        bound_classes_.insert(class_name);

        return *this;
    }

    template <typename... Classes> inline PyGenerator &PyGenerator::bind_classes() {
        (bind_class<Classes>(), ...);
        return *this;
    }

    template <typename Ret, typename... Args>
    inline PyGenerator &PyGenerator::bind_function(const std::string &name, Ret (*func)(Args...),
                                                   const std::string &docstring) {
        // Use pybind11's automatic function binding
        module.def(name.c_str(), func, docstring.c_str());
        return *this;
    }

    inline PyGenerator &PyGenerator::add_utilities() {
        // Add version info
        module.def(
            "get_version", []() { return "1.0.0"; }, "Get Rosetta version");

        // Add class list
        module.def(
            "list_classes",
            []() {
                auto &registry = core::Registry::instance();
                return registry.list_classes();
            },
            "List all registered classes");

        return *this;
    }

    inline PyGenerator &PyGenerator::set_module_doc(const std::string &doc) {
        module.doc() = doc.c_str();
        return *this;
    }

    // ========================================================================
    // STANDALONE HELPERS
    // ========================================================================

    template <typename T>
    inline void bind_class(PyGenerator &generator, const std::string &class_name) {
        generator.bind_class<T>(class_name);
    }

    template <typename... Classes> inline void bind_classes(PyGenerator &generator) {
        generator.bind_classes<Classes...>();
    }

} // namespace rosetta::generators::python