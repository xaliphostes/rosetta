namespace py = pybind11;

namespace rosetta::bindings {

    // Forward declarations
    class TypeConverterRegistry;
    class TypeCastRegistry;

    // ============================================================================
    // Type Converter Registry - Python to C++
    // ============================================================================

    /**
     * @brief Type converter registry for custom classes
     * Maps type_index to converter functions that convert Python objects to C++ Any
     */
    class TypeConverterRegistry {
    private:
        using ConverterFunc = std::function<core::Any(const py::object &)>;
        std::unordered_map<std::type_index, ConverterFunc> converters_;

        TypeConverterRegistry() = default;

    public:
        static TypeConverterRegistry &instance() {
            static TypeConverterRegistry registry;
            return registry;
        }

        template <typename T> void register_converter() {
            converters_[std::type_index(typeid(T))] = [](const py::object &obj) -> core::Any {
                try {
                    // Try to cast the Python object to the C++ type
                    // pybind11 will handle the conversion for registered types
                    T cpp_obj = obj.cast<T>();
                    return core::Any(cpp_obj);
                } catch (const py::cast_error &e) {
                    throw std::runtime_error(
                        std::string("Failed to convert Python object to C++ type: ") + e.what());
                }
            };
        }

        bool has_converter(std::type_index type) const {
            return converters_.find(type) != converters_.end();
        }

        core::Any convert(const py::object &obj, std::type_index type) const {
            auto it = converters_.find(type);
            if (it != converters_.end()) {
                return it->second(obj);
            }
            return core::Any();
        }
    };

    // ============================================================================
    // Type Cast Registry - C++ to Python
    // ============================================================================

    /**
     * @brief Registry for type casting functions from C++ Any to Python
     * Maps type_index to functions that convert C++ objects (stored in Any) to py::object
     */
    class TypeCastRegistry {
    private:
        using CastFunc = std::function<py::object(const core::Any &)>;
        std::unordered_map<std::type_index, CastFunc> cast_funcs_;

        TypeCastRegistry() = default;

    public:
        static TypeCastRegistry &instance() {
            static TypeCastRegistry registry;
            return registry;
        }

        template <typename T> void register_cast() {
            cast_funcs_[std::type_index(typeid(T))] = [](const core::Any &value) -> py::object {
                try {
                    T obj = value.as<T>();
                    return py::cast(obj);
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Failed to cast C++ object to Python: ") +
                                             e.what());
                }
            };
        }

        bool has_cast(std::type_index type) const {
            return cast_funcs_.find(type) != cast_funcs_.end();
        }

        py::object cast_to_python(const core::Any &value) const {
            auto type = value.get_type_index();
            auto it   = cast_funcs_.find(type);
            if (it != cast_funcs_.end()) {
                return it->second(value);
            }
            return py::none();
        }
    };

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

        // Try custom type cast registry
        auto &registry = TypeCastRegistry::instance();
        if (registry.has_cast(type)) {
            return registry.cast_to_python(value);
        }

        // For types we can't convert, return None
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

        // Handle primitives first
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

        // Try custom type converters for registered classes
        auto &registry = TypeConverterRegistry::instance();
        if (registry.has_converter(expected_type)) {
            return registry.convert(py_obj, expected_type);
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

            // Register type converter for this class (Python -> C++)
            TypeConverterRegistry::instance().register_converter<T>();

            // Register type cast function for this class (C++ -> Python)
            TypeCastRegistry::instance().register_cast<T>();

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
            // Check if we have constructor_infos() method (new API)
            if constexpr (requires { meta.constructor_infos(); }) {
                const auto &ctor_infos = meta.constructor_infos();

                if (ctor_infos.empty()) {
                    // No registered constructors, try default
                    if constexpr (std::is_default_constructible_v<T>) {
                        py_class.def(py::init<>());
                    }
                    return;
                }

                // Bind each constructor with known parameter types
                for (const auto &ctor_info : ctor_infos) {
                    if (ctor_info.arity == 0) {
                        // Default constructor
                        py_class.def(py::init([ctor = ctor_info.invoker]() {
                            std::vector<core::Any> args;
                            core::Any              result = ctor(args);
                            return result.as<T>();
                        }));
                    } else {
                        // Parametric constructor with known types
                        bind_typed_constructor(py_class, ctor_info);
                    }
                }
            } else {
                // Fallback to old API without type information
                const auto &ctors = meta.constructors();

                if (ctors.empty()) {
                    if constexpr (std::is_default_constructible_v<T>) {
                        py_class.def(py::init<>());
                    }
                    return;
                }

                for (const auto &ctor : ctors) {
                    int arity = detect_constructor_arity(ctor);
                    if (arity == 0) {
                        py_class.def(py::init([ctor]() {
                            std::vector<core::Any> args;
                            core::Any              result = ctor(args);
                            return result.as<T>();
                        }));
                    } else if (arity > 0) {
                        bind_parametric_constructor(py_class, ctor, arity);
                    }
                }
            }
        }

        /**
         * @brief Bind a constructor with known parameter types
         */
        template <typename CtorInfo>
        static void bind_typed_constructor(py::class_<T> &py_class, const CtorInfo &ctor_info) {
            py_class.def(py::init([ctor = ctor_info.invoker, param_types = ctor_info.param_types,
                                   arity = ctor_info.arity](py::args args) {
                if (args.size() != arity) {
                    throw std::runtime_error("Constructor expects " + std::to_string(arity) +
                                             " arguments, got " + std::to_string(args.size()));
                }

                // Convert Python args to C++ Any using the actual parameter types
                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    py::object arg = args[i];
                    // Use the actual expected type from constructor signature
                    cpp_args.push_back(python_to_any(arg, param_types[i]));
                }

                core::Any result = ctor(cpp_args);
                return result.as<T>();
            }));
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

    inline PyBindingGenerator::PyBindingGenerator(py::module_ &m) : module_(m) {
    }

    template <typename T>
    inline PyBindingGenerator &PyBindingGenerator::bind_class(const std::string &py_name) {
        PyClassBinder<T>::bind(module_, py_name);
        return *this;
    }

    inline PyBindingGenerator &PyBindingGenerator::add_utilities() {
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

    inline PyBindingGenerator &PyBindingGenerator::set_doc(const std::string &doc) {
        module_.doc() = doc.c_str();
        return *this;
    }

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> inline void bind_classes(PyBindingGenerator &gen) {
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