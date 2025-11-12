// ============================================================================
// rosetta/generators/python_binder_unified.hxx
//
// Unified implementation - no code duplication!
// ============================================================================

#include <rosetta/core/registry.h>
#include <rosetta/core/version.h>
#include <stdexcept>
#include <algorithm>

namespace rosetta::python {

    // ============================================================================
    // Type Converter Implementation
    // ============================================================================

    inline TypeConverter::ConverterRegistry& TypeConverter::ConverterRegistry::instance() {
        static ConverterRegistry registry;
        return registry;
    }

    inline py::object TypeConverter::any_to_python(const core::Any& value) {
        if (!value.has_value()) {
            return py::none();
        }

        auto type = value.get_type_index();
        
        // Check custom converters first
        auto& registry = ConverterRegistry::instance();
        auto it = registry.to_python.find(type);
        if (it != registry.to_python.end()) {
            return it->second(value);
        }

        // Built-in type conversions
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
        
        // Common STL containers
        if (type == std::type_index(typeid(std::vector<int>))) {
            return py::cast(value.as<std::vector<int>>());
        }
        if (type == std::type_index(typeid(std::vector<double>))) {
            return py::cast(value.as<std::vector<double>>());
        }
        if (type == std::type_index(typeid(std::vector<float>))) {
            return py::cast(value.as<std::vector<float>>());
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            return py::cast(value.as<std::vector<std::string>>());
        }
        if (type == std::type_index(typeid(std::vector<bool>))) {
            return py::cast(value.as<std::vector<bool>>());
        }
        
        // Maps
        if (type == std::type_index(typeid(std::map<std::string, int>))) {
            return py::cast(value.as<std::map<std::string, int>>());
        }
        if (type == std::type_index(typeid(std::map<std::string, double>))) {
            return py::cast(value.as<std::map<std::string, double>>());
        }
        if (type == std::type_index(typeid(std::map<std::string, std::string>))) {
            return py::cast(value.as<std::map<std::string, std::string>>());
        }

        // Unknown type - return None
        return py::none();
    }

    inline core::Any TypeConverter::python_to_any(const py::object& obj, std::type_index expected_type) {
        if (obj.is_none()) {
            return core::Any();
        }
        
        // Check custom converters first
        auto& registry = ConverterRegistry::instance();
        auto it = registry.to_cpp.find(expected_type);
        if (it != registry.to_cpp.end()) {
            return it->second(obj);
        }

        // Built-in conversions based on expected type
        if (expected_type == std::type_index(typeid(int))) {
            return core::Any(obj.cast<int>());
        }
        if (expected_type == std::type_index(typeid(double))) {
            return core::Any(obj.cast<double>());
        }
        if (expected_type == std::type_index(typeid(float))) {
            return core::Any(obj.cast<float>());
        }
        if (expected_type == std::type_index(typeid(bool))) {
            return core::Any(obj.cast<bool>());
        }
        if (expected_type == std::type_index(typeid(std::string))) {
            return core::Any(obj.cast<std::string>());
        }
        if (expected_type == std::type_index(typeid(size_t))) {
            return core::Any(obj.cast<size_t>());
        }
        if (expected_type == std::type_index(typeid(long))) {
            return core::Any(obj.cast<long>());
        }
        
        // STL containers
        if (expected_type == std::type_index(typeid(std::vector<int>))) {
            return core::Any(obj.cast<std::vector<int>>());
        }
        if (expected_type == std::type_index(typeid(std::vector<double>))) {
            return core::Any(obj.cast<std::vector<double>>());
        }
        if (expected_type == std::type_index(typeid(std::vector<float>))) {
            return core::Any(obj.cast<std::vector<float>>());
        }
        if (expected_type == std::type_index(typeid(std::vector<std::string>))) {
            return core::Any(obj.cast<std::vector<std::string>>());
        }
        if (expected_type == std::type_index(typeid(std::map<std::string, int>))) {
            return core::Any(obj.cast<std::map<std::string, int>>());
        }
        if (expected_type == std::type_index(typeid(std::map<std::string, double>))) {
            return core::Any(obj.cast<std::map<std::string, double>>());
        }
        if (expected_type == std::type_index(typeid(std::map<std::string, std::string>))) {
            return core::Any(obj.cast<std::map<std::string, std::string>>());
        }

        // Fallback: return empty
        return core::Any();
    }

    inline core::Any TypeConverter::python_to_any(const py::object& obj) {
        if (obj.is_none()) {
            return core::Any();
        }
        
        // Auto-detect type from Python object
        // Note: Check bool BEFORE int (bool is subclass of int in Python)
        if (py::isinstance<py::bool_>(obj)) {
            return core::Any(obj.cast<bool>());
        }
        if (py::isinstance<py::int_>(obj)) {
            return core::Any(obj.cast<int>());
        }
        if (py::isinstance<py::float_>(obj)) {
            return core::Any(obj.cast<double>());
        }
        if (py::isinstance<py::str>(obj)) {
            return core::Any(obj.cast<std::string>());
        }
        if (py::isinstance<py::list>(obj)) {
            // Try to infer list type from first element
            auto list = obj.cast<py::list>();
            if (list.size() > 0) {
                auto first = list[0];
                if (py::isinstance<py::int_>(first)) {
                    return core::Any(obj.cast<std::vector<int>>());
                }
                if (py::isinstance<py::float_>(first)) {
                    return core::Any(obj.cast<std::vector<double>>());
                }
                if (py::isinstance<py::str>(first)) {
                    return core::Any(obj.cast<std::vector<std::string>>());
                }
            }
        }
        if (py::isinstance<py::dict>(obj)) {
            // Try to infer dict type
            return core::Any(obj.cast<std::map<std::string, std::string>>());
        }

        return core::Any();
    }

    template <typename T>
    inline void TypeConverter::register_type() {
        auto& registry = ConverterRegistry::instance();
        
        // Register C++ to Python converter
        registry.to_python[std::type_index(typeid(T))] = 
            [](const core::Any& value) -> py::object {
                try {
                    return py::cast(value.as<T>());
                } catch (const std::exception& e) {
                    throw std::runtime_error(
                        std::string("Failed to convert C++ type to Python: ") + e.what()
                    );
                }
            };
        
        // Register Python to C++ converter
        registry.to_cpp[std::type_index(typeid(T))] = 
            [](const py::object& obj) -> core::Any {
                try {
                    return core::Any(obj.cast<T>());
                } catch (const py::cast_error& e) {
                    throw std::runtime_error(
                        std::string("Failed to convert Python type to C++: ") + e.what()
                    );
                }
            };
    }

    // ============================================================================
    // Class Binder Implementation (shared by both manual and automatic binding)
    // ============================================================================

    template <typename Class>
    inline py::class_<Class> ClassBinder<Class>::bind(py::module& m, const std::string& name) {
        // Get metadata from registry
        auto& meta = core::Registry::instance().get<Class>();
        
        // Use provided name or fall back to registered name
        std::string class_name = name.empty() ? meta.name() : name;
        
        // Create Python class
        py::class_<Class> py_class(m, class_name.c_str());
        
        // Bind all components
        bind_constructors(py_class, meta);
        bind_fields(py_class, meta);
        bind_methods(py_class, meta);
        bind_static_methods(py_class, meta);
        
        // Add __repr__
        py_class.def("__repr__", [class_name](const Class&) {
            return "<" + class_name + " object>";
        });
        
        return py_class;
    }

    template <typename Class>
    inline void ClassBinder<Class>::bind_constructors(
        py::class_<Class>& py_class,
        const core::ClassMetadata<Class>& meta
    ) {
        const auto& ctor_infos = meta.constructor_infos();
        
        for (const auto& ctor_info : ctor_infos) {
            if (ctor_info.arity == 0) {
                py_class.def(py::init<>());
            } else {
                // Generic constructor with runtime argument conversion
                py_class.def(py::init([ctor_info](py::args args) {
                    if (args.size() != ctor_info.arity) {
                        throw std::runtime_error(
                            "Constructor expects " + std::to_string(ctor_info.arity) +
                            " arguments, got " + std::to_string(args.size())
                        );
                    }
                    
                    std::vector<core::Any> cpp_args;
                    cpp_args.reserve(args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        cpp_args.push_back(
                            TypeConverter::python_to_any(args[i], ctor_info.param_types[i])
                        );
                    }
                    
                    core::Any result = ctor_info.invoker(cpp_args);
                    return result.as<Class>();
                }));
            }
        }
        
        // Fallback: default constructor if instantiable and no constructors registered
        if (ctor_infos.empty() && meta.is_instantiable()) {
            py_class.def(py::init<>());
        }
    }

    template <typename Class>
    inline void ClassBinder<Class>::bind_fields(
        py::class_<Class>& py_class,
        const core::ClassMetadata<Class>& meta
    ) {
        const auto& field_names = meta.fields();
        
        for (const auto& field_name : field_names) {
            std::type_index field_type = meta.get_field_type(field_name);
            
            py_class.def_property(
                field_name.c_str(),
                // Getter
                [field_name](Class& obj) -> py::object {
                    auto& meta = core::Registry::instance().get<Class>();
                    core::Any value = meta.get_field(obj, field_name);
                    return TypeConverter::any_to_python(value);
                },
                // Setter
                [field_name, field_type](Class& obj, py::object value) {
                    auto& meta = core::Registry::instance().get<Class>();
                    core::Any cpp_value = TypeConverter::python_to_any(value, field_type);
                    meta.set_field(obj, field_name, cpp_value);
                }
            );
        }
    }

    template <typename Class>
    inline void ClassBinder<Class>::bind_methods(
        py::class_<Class>& py_class,
        const core::ClassMetadata<Class>& meta
    ) {
        const auto& method_names = meta.methods();
        
        for (const auto& method_name : method_names) {
            // Skip static methods
            if (meta.is_static_method(method_name)) {
                continue;
            }
            
            size_t arity = meta.get_method_arity(method_name);
            const auto& arg_types = meta.get_method_arg_types(method_name);
            
            py_class.def(
                method_name.c_str(),
                [method_name, arity, arg_types](Class& obj, py::args args) -> py::object {
                    if (args.size() != arity) {
                        throw std::runtime_error(
                            "Method '" + method_name + "' expects " +
                            std::to_string(arity) + " arguments, got " +
                            std::to_string(args.size())
                        );
                    }
                    
                    std::vector<core::Any> cpp_args;
                    cpp_args.reserve(args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        cpp_args.push_back(
                            TypeConverter::python_to_any(args[i], arg_types[i])
                        );
                    }
                    
                    auto& meta = core::Registry::instance().get<Class>();
                    core::Any result = meta.invoke_method(obj, method_name, cpp_args);
                    return TypeConverter::any_to_python(result);
                }
            );
        }
    }

    template <typename Class>
    inline void ClassBinder<Class>::bind_static_methods(
        py::class_<Class>& py_class,
        const core::ClassMetadata<Class>& meta
    ) {
        const auto& method_names = meta.methods();
        
        for (const auto& method_name : method_names) {
            // Only bind static methods
            if (!meta.is_static_method(method_name)) {
                continue;
            }
            
            size_t arity = meta.get_method_arity(method_name);
            const auto& arg_types = meta.get_method_arg_types(method_name);
            
            py_class.def_static(
                method_name.c_str(),
                [method_name, arity, arg_types](py::args args) -> py::object {
                    if (args.size() != arity) {
                        throw std::runtime_error(
                            "Static method '" + method_name + "' expects " +
                            std::to_string(arity) + " arguments, got " +
                            std::to_string(args.size())
                        );
                    }
                    
                    std::vector<core::Any> cpp_args;
                    cpp_args.reserve(args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        cpp_args.push_back(
                            TypeConverter::python_to_any(args[i], arg_types[i])
                        );
                    }
                    
                    auto& meta = core::Registry::instance().get<Class>();
                    core::Any result = meta.invoke_static_method(method_name, cpp_args);
                    return TypeConverter::any_to_python(result);
                }
            );
        }
    }

    // ============================================================================
    // Binder Registry Implementation
    // ============================================================================

    inline BinderRegistry& BinderRegistry::instance() {
        static BinderRegistry registry;
        return registry;
    }

    template <typename Class>
    inline void BinderRegistry::register_class(const std::string& name) {
        binders_[name] = std::make_unique<TypedBinder<Class>>(name);
        std::cerr << "--> register py class " << name << std::endl;
    }

    inline void BinderRegistry::bind_all(
        py::module& m,
        std::function<bool(const std::string&)> filter
    ) {
        for (auto& [name, binder] : binders_) {
            std::cerr << "--> try to bind class " << name ;
            if (filter && !filter(name)) {
                std::cerr << "nop! " << std::endl ;
                continue;
            }
            std::cerr << "yes! " << std::endl ;
            binder->bind(m);
        }
    }

    inline bool BinderRegistry::has_class(const std::string& name) const {
        return binders_.find(name) != binders_.end();
    }

    inline void BinderRegistry::clear() {
        binders_.clear();
    }

    // ============================================================================
    // Python Binder Implementation (main API)
    // ============================================================================

    inline PythonBinder::PythonBinder(py::module& m) : module_(m) {}

    template <typename Class>
    inline PythonBinder& PythonBinder::bind_class(const std::string& name) {
        ClassBinder<Class>::bind(module_, name);
        return *this;
    }

    template <typename Ret, typename... Args>
    inline PythonBinder& PythonBinder::bind_function(
        const std::string& name,
        Ret (*func)(Args...),
        const std::string& docstring
    ) {
        module_.def(name.c_str(), func, docstring.c_str());
        return *this;
    }

    template <typename T>
    inline PythonBinder& PythonBinder::bind_constant(const std::string& name, T value) {
        module_.attr(name.c_str()) = value;
        return *this;
    }

    inline PythonBinder& PythonBinder::bind_all_registered(
        std::function<bool(const std::string&)> filter
    ) {
        BinderRegistry::instance().bind_all(module_, filter);
        return *this;
    }

    inline PythonBinder& PythonBinder::add_utilities() {
        // List all registered classes
        module_.def(
            "list_classes",
            []() { return core::Registry::instance().list_classes(); },
            "List all registered classes"
        );

        // Get class information
        module_.def(
            "get_class_info",
            [](const std::string& name) -> py::dict {
                auto holder = core::Registry::instance().get_by_name(name);
                if (!holder) {
                    throw std::runtime_error("Class not found: " + name);
                }

                py::dict info;
                info["name"] = holder->get_name();

                const auto& inh = holder->get_inheritance();
                info["is_abstract"] = inh.is_abstract;
                info["is_polymorphic"] = inh.is_polymorphic;
                info["has_virtual_destructor"] = inh.has_virtual_destructor;
                info["base_count"] = inh.total_base_count();

                return info;
            },
            py::arg("name"),
            "Get information about a registered class"
        );

        // Version info
        module_.def(
            "version",
            []() { return rosetta::core::version(); },
            "Get Rosetta version"
        );

        return *this;
    }

    inline PythonBinder& PythonBinder::set_doc(const std::string& doc) {
        module_.doc() = doc.c_str();
        return *this;
    }

} // namespace rosetta::python