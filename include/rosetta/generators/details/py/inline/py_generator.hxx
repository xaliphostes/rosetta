/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
namespace rosetta {

    template <typename T>
    inline auto PyGenerator::bind_class(const std::string& class_name) -> py::class_<T>
    {
        static_assert(
            std::is_base_of_v<Introspectable, T>, "Type must inherit from Introspectable");

        // Get type info from introspection system
        T dummy_instance;
        const auto& type_info = dummy_instance.getTypeInfo();

        std::string final_class_name = class_name.empty() ? type_info.class_name : class_name;

        // Prevent duplicate bindings
        if (bound_classes.find(final_class_name) != bound_classes.end()) {
            throw std::runtime_error("Class '" + final_class_name + "' already bound");
        }
        bound_classes.insert(final_class_name);

        // Create pybind11 class
        auto py_class = py::class_<T>(module, final_class_name.c_str());

        // Bind constructors (we may want to customize this)
        bind_constructors<T>(py_class, type_info);

        // Bind all members as properties
        bind_members<T>(py_class, type_info);

        // Bind all methods
        bind_methods<T>(py_class, type_info);

        // Add introspection utilities to Python
        bind_introspection_utilities<T>(py_class);

        return py_class;
    }

    template <typename... Classes> inline void PyGenerator::bind_classes()
    {
        (bind_class<Classes>(), ...);
    }

    template <typename T>
    inline void PyGenerator::bind_constructors(py::class_<T>& py_class, const TypeInfo& type_info)
    {
        const auto& constructors = type_info.getConstructors();

        if (constructors.empty()) {
            py_class.def(py::init<>(), "Default constructor");
            return;
        }

        // Bind each constructor generically
        for (const auto& ctor : constructors) {
            py_class.def(py::init([this, ctor_ptr = ctor.get()](py::args args) -> T* {
                if (args.size() != ctor_ptr->parameter_types.size()) {
                    throw py::value_error("Constructor expects "
                        + std::to_string(ctor_ptr->parameter_types.size()) + " arguments, got "
                        + std::to_string(args.size()));
                }

                // Convert all Python arguments to C++ std::any
                std::vector<std::any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    cpp_args.push_back(
                        convert_python_to_any(args[i], ctor_ptr->parameter_types[i]));
                }

                // Call factory to create object
                void* raw_ptr = ctor_ptr->factory(cpp_args);
                return static_cast<T*>(raw_ptr);
            }));
        }
    }

    template <typename T>
    inline void PyGenerator::bind_members(py::class_<T>& py_class, const TypeInfo& type_info)
    {
        for (const auto& member_name : type_info.getMemberNames()) {
            const auto* member = type_info.getMember(member_name);
            if (!member)
                continue;

            // Create Python property using introspection getter/setter
            py_class.def_property(
                member_name.c_str(),
                // Getter
                [this, member_name](const T& obj) -> py::object {
                    try {
                        auto value = obj.getMemberValue(member_name);
                        return convert_any_to_python(
                            value, obj.getTypeInfo().getMember(member_name)->type_name);
                    } catch (const std::exception& e) {
                        throw py::value_error(
                            "Failed to get member '" + member_name + "': " + e.what());
                    }
                },
                // Setter
                [this, member_name](T& obj, py::object py_value) {
                    try {
                        const auto* member_info = obj.getTypeInfo().getMember(member_name);
                        auto cpp_value = convert_python_to_any(py_value, member_info->type_name);
                        obj.setMemberValue(member_name, cpp_value);
                    } catch (const std::exception& e) {
                        throw py::value_error(
                            "Failed to set member '" + member_name + "': " + e.what());
                    }
                },
                ("Access to " + member_name + " member").c_str());
        }
    }

    template <typename T>
    inline void PyGenerator::bind_methods(py::class_<T>& py_class, const TypeInfo& type_info)
    {
        for (const auto& method_name : type_info.getMethodNames()) {
            const auto* method = type_info.getMethod(method_name);
            if (!method)
                continue;

            // Skip getter/setter methods that are already bound as properties
            if (is_getter_setter_method(method_name))
                continue;

            // Create Python method using introspection
            py_class.def(
                method_name.c_str(),
                [this, method_name](T& obj, py::args args) -> py::object {
                    try {
                        // Convert Python arguments to std::any vector
                        std::vector<std::any> cpp_args;
                        const auto* method_info = obj.getTypeInfo().getMethod(method_name);

                        if (args.size() != method_info->parameter_types.size()) {
                            throw py::value_error("Method '" + method_name + "' expects "
                                + std::to_string(method_info->parameter_types.size())
                                + " arguments, got " + std::to_string(args.size()));
                        }

                        for (size_t i = 0; i < args.size(); ++i) {
                            cpp_args.push_back(
                                convert_python_to_any(args[i], method_info->parameter_types[i]));
                        }

                        // Call method through introspection
                        auto result = obj.callMethod(method_name, cpp_args);

                        // Convert result back to Python
                        return convert_any_to_python(result, method_info->return_type);
                    } catch (const std::exception& e) {
                        // throw py::runtime_error("Failed to call method '" +
                        //                         method_name + "': " + e.what());
                    }
                },
                ("Call " + method_name + " method").c_str());
        }
    }

    template <typename T>
    inline void PyGenerator::bind_introspection_utilities(py::class_<T>& py_class)
    {
        // Expose introspection utilities to Python
        py_class.def("get_class_name", &T::getClassName, "Get the class name");
        py_class.def("get_member_names", &T::getMemberNames, "Get all member names");
        py_class.def("get_method_names", &T::getMethodNames, "Get all method names");
        py_class.def("has_member", &T::hasMember, "Check if member exists");
        py_class.def("has_method", &T::hasMethod, "Check if method exists");
        py_class.def("to_json", &T::toJSON, "Export object to JSON string");

        // Dynamic member/method access
        py_class.def(
            "get_member_value",
            [this](const T& obj, const std::string& name) -> py::object {
                auto value = obj.getMemberValue(name);
                const auto* member = obj.getTypeInfo().getMember(name);
                return convert_any_to_python(value, member ? member->type_name : "unknown");
            },
            "Get member value by name");

        py_class.def(
            "set_member_value",
            [this](T& obj, const std::string& name, py::object value) {
                const auto* member = obj.getTypeInfo().getMember(name);
                if (!member)
                    throw py::value_error("Member not found: " + name);
                auto cpp_value = convert_python_to_any(value, member->type_name);
                obj.setMemberValue(name, cpp_value);
            },
            "Set member value by name");

        py_class.def(
            "call_method",
            [this](T& obj, const std::string& name, py::list args) -> py::object {
                std::vector<std::any> cpp_args;
                const auto* method = obj.getTypeInfo().getMethod(name);
                if (!method)
                    throw py::value_error("Method not found: " + name);

                for (size_t i = 0; i < args.size(); ++i) {
                    if (i < method->parameter_types.size()) {
                        cpp_args.push_back(
                            convert_python_to_any(args[i], method->parameter_types[i]));
                    }
                }

                auto result = obj.callMethod(name, cpp_args);
                return convert_any_to_python(result, method->return_type);
            },
            "Call method by name with arguments");
    }

    // Helper function to check if a method is a getter/setter
    inline bool PyGenerator::is_getter_setter_method(const std::string& method_name) const
    {
        return method_name.starts_with("get") || method_name.starts_with("set")
            || method_name.starts_with("is");
    }

    // Convert std::any to Python object based on type name
    inline py::object PyGenerator::convert_any_to_python(
        const std::any& value, const std::string& type_name) const
    {
        if (value.has_value() == false || type_name == "void") {
            return py::none();
        }

        try {
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
            } else {
                // For custom types, try generic casting
                // we may need to extend this for our custom types
                return py::cast(value);
            }
        } catch (const std::bad_any_cast& e) {
            throw py::type_error(
                "Failed to convert type '" + type_name + "' to Python: " + e.what());
        }
    }

    // Convert Python object to std::any based on expected type
    inline std::any PyGenerator::convert_python_to_any(
        py::object py_value, const std::string& type_name) const
    {
        try {
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
            } else {
                // For custom types, this is more complex and would require
                // additional type information or registration
                throw py::type_error("Unsupported type conversion for: " + type_name);
            }
        } catch (const py::cast_error& e) {
            throw py::type_error(
                "Failed to convert Python object to '" + type_name + "': " + e.what());
        }
    }

}
