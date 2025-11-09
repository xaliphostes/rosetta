#include <map>
#include <set>
#include <array>

namespace py = pybind11;

namespace rosetta::py {

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
        using ConverterFunc = std::function<core::Any(const pyb11::object &)>;
        std::unordered_map<std::type_index, ConverterFunc> converters_;

        TypeConverterRegistry() = default;

    public:
        static TypeConverterRegistry &instance() {
            static TypeConverterRegistry registry;
            return registry;
        }

        template <typename T> void register_converter() {
            converters_[std::type_index(typeid(T))] = [](const pyb11::object &obj) -> core::Any {
                try {
                    // Try to cast the Python object to the C++ type
                    // pybind11 will handle the conversion for registered types
                    T cpp_obj = obj.cast<T>();
                    return core::Any(cpp_obj);
                } catch (const pyb11::cast_error &e) {
                    throw std::runtime_error(
                        std::string("Failed to convert Python object to C++ type: ") + e.what());
                }
            };
        }

        /**
         * @brief Register a custom converter function
         * @param type_index Type to register converter for
         * @param converter Converter function
         */
        void register_custom_converter(std::type_index type_index, ConverterFunc converter) {
            converters_[type_index] = std::move(converter);
        }

        bool has_converter(std::type_index type) const {
            return converters_.find(type) != converters_.end();
        }

        core::Any convert(const pyb11::object &obj, std::type_index type) const {
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
     * Maps type_index to functions that convert C++ objects (stored in Any) to pyb11::object
     */
    class TypeCastRegistry {
    private:
        using CastFunc = std::function<pyb11::object(const core::Any &)>;
        std::unordered_map<std::type_index, CastFunc> cast_funcs_;

        TypeCastRegistry() = default;

    public:
        static TypeCastRegistry &instance() {
            static TypeCastRegistry registry;
            return registry;
        }

        template <typename T> void register_cast() {
            cast_funcs_[std::type_index(typeid(T))] = [](const core::Any &value) -> pyb11::object {
                try {
                    T obj = value.as<T>();
                    return pyb11::cast(obj);
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Failed to cast C++ object to Python: ") +
                                             e.what());
                }
            };
        }

        bool has_cast(std::type_index type) const {
            return cast_funcs_.find(type) != cast_funcs_.end();
        }

        pyb11::object cast_to_python(const core::Any &value) const {
            auto type = value.get_type_index();
            auto it   = cast_funcs_.find(type);
            if (it != cast_funcs_.end()) {
                return it->second(value);
            }
            return pyb11::none();
        }
    };

    // ============================================================================
    // Type Converters - Convert between C++ Any and Python objects
    // ============================================================================

    /**
     * @brief Convert C++ Any to Python object
     */
    inline pyb11::object any_to_python(const core::Any &value) {
        if (!value.has_value()) {
            return pyb11::none();
        }

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) {
            return pyb11::cast(value.as<int>());
        }
        if (type == std::type_index(typeid(double))) {
            return pyb11::cast(value.as<double>());
        }
        if (type == std::type_index(typeid(float))) {
            return pyb11::cast(value.as<float>());
        }
        if (type == std::type_index(typeid(bool))) {
            return pyb11::cast(value.as<bool>());
        }
        if (type == std::type_index(typeid(std::string))) {
            return pyb11::cast(value.as<std::string>());
        }
        if (type == std::type_index(typeid(size_t))) {
            return pyb11::cast(value.as<size_t>());
        }
        if (type == std::type_index(typeid(long))) {
            return pyb11::cast(value.as<long>());
        }
        if (type == std::type_index(typeid(long long))) {
            return pyb11::cast(value.as<long long>());
        }

        // Void return (method that returns nothing)
        if (type == std::type_index(typeid(void))) {
            return pyb11::none();
        }

        // Handle std::vector<T> conversions to Python lists
        if (type == std::type_index(typeid(std::vector<double>))) {
            return pyb11::cast(value.as<std::vector<double>>());
        }
        if (type == std::type_index(typeid(std::vector<float>))) {
            return pyb11::cast(value.as<std::vector<float>>());
        }
        if (type == std::type_index(typeid(std::vector<int>))) {
            return pyb11::cast(value.as<std::vector<int>>());
        }
        if (type == std::type_index(typeid(std::vector<size_t>))) {
            return pyb11::cast(value.as<std::vector<size_t>>());
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            return pyb11::cast(value.as<std::vector<std::string>>());
        }
        if (type == std::type_index(typeid(std::vector<bool>))) {
            return pyb11::cast(value.as<std::vector<bool>>());
        }

        // Handle std::map<K,V> conversions to Python dicts
        if (type == std::type_index(typeid(std::map<std::string, int>))) {
            return pyb11::cast(value.as<std::map<std::string, int>>());
        }
        if (type == std::type_index(typeid(std::map<std::string, double>))) {
            return pyb11::cast(value.as<std::map<std::string, double>>());
        }
        if (type == std::type_index(typeid(std::map<std::string, std::string>))) {
            return pyb11::cast(value.as<std::map<std::string, std::string>>());
        }
        if (type == std::type_index(typeid(std::map<int, int>))) {
            return pyb11::cast(value.as<std::map<int, int>>());
        }
        if (type == std::type_index(typeid(std::map<int, double>))) {
            return pyb11::cast(value.as<std::map<int, double>>());
        }
        if (type == std::type_index(typeid(std::map<int, std::string>))) {
            return pyb11::cast(value.as<std::map<int, std::string>>());
        }

        // Handle std::set<T> conversions to Python sets
        if (type == std::type_index(typeid(std::set<int>))) {
            return pyb11::cast(value.as<std::set<int>>());
        }
        if (type == std::type_index(typeid(std::set<double>))) {
            return pyb11::cast(value.as<std::set<double>>());
        }
        if (type == std::type_index(typeid(std::set<std::string>))) {
            return pyb11::cast(value.as<std::set<std::string>>());
        }

        // Handle std::array<T, N> conversions to Python lists
        // Note: We need to add the most common sizes
        if (type == std::type_index(typeid(std::array<double, 2>))) {
            return pyb11::cast(value.as<std::array<double, 2>>());
        }
        if (type == std::type_index(typeid(std::array<double, 3>))) {
            return pyb11::cast(value.as<std::array<double, 3>>());
        }
        if (type == std::type_index(typeid(std::array<double, 4>))) {
            return pyb11::cast(value.as<std::array<double, 4>>());
        }
        if (type == std::type_index(typeid(std::array<int, 2>))) {
            return pyb11::cast(value.as<std::array<int, 2>>());
        }
        if (type == std::type_index(typeid(std::array<int, 3>))) {
            return pyb11::cast(value.as<std::array<int, 3>>());
        }
        if (type == std::type_index(typeid(std::array<int, 4>))) {
            return pyb11::cast(value.as<std::array<int, 4>>());
        }

        // Try custom type cast registry
        auto &registry = TypeCastRegistry::instance();
        if (registry.has_cast(type)) {
            return registry.cast_to_python(value);
        }

        // For types we can't convert, return None
        return pyb11::none();
    }

    /**
     * @brief Convert Python object to C++ Any
     */
    inline core::Any python_to_any(const pyb11::object &py_obj, std::type_index expected_type) {
        // Handle None
        if (py_obj.is_none()) {
            return core::Any();
        }

        // Handle primitives first
        if (expected_type == std::type_index(typeid(int))) {
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(py_obj.cast<int>());
            }
        }
        if (expected_type == std::type_index(typeid(double))) {
            if (pyb11::isinstance<pyb11::float_>(py_obj)) {
                return core::Any(py_obj.cast<double>());
            }
            // Python int can convert to double
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(static_cast<double>(py_obj.cast<int>()));
            }
        }
        if (expected_type == std::type_index(typeid(float))) {
            if (pyb11::isinstance<pyb11::float_>(py_obj)) {
                return core::Any(static_cast<float>(py_obj.cast<double>()));
            }
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(static_cast<float>(py_obj.cast<int>()));
            }
        }
        if (expected_type == std::type_index(typeid(bool))) {
            if (pyb11::isinstance<pyb11::bool_>(py_obj)) {
                return core::Any(py_obj.cast<bool>());
            }
        }
        if (expected_type == std::type_index(typeid(std::string))) {
            if (pyb11::isinstance<pyb11::str>(py_obj)) {
                return core::Any(py_obj.cast<std::string>());
            }
        }
        if (expected_type == std::type_index(typeid(size_t))) {
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(py_obj.cast<size_t>());
            }
        }
        if (expected_type == std::type_index(typeid(long))) {
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(py_obj.cast<long>());
            }
        }
        if (expected_type == std::type_index(typeid(long long))) {
            if (pyb11::isinstance<pyb11::int_>(py_obj)) {
                return core::Any(py_obj.cast<long long>());
            }
        }

        // Handle std::vector<T> conversions
        // std::vector<double>
        if (expected_type == std::type_index(typeid(std::vector<double>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<double>>());
            }
        }
        // std::vector<float>
        if (expected_type == std::type_index(typeid(std::vector<float>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<float>>());
            }
        }
        // std::vector<int>
        if (expected_type == std::type_index(typeid(std::vector<int>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<int>>());
            }
        }
        // std::vector<size_t>
        if (expected_type == std::type_index(typeid(std::vector<size_t>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<size_t>>());
            }
        }
        // std::vector<std::string>
        if (expected_type == std::type_index(typeid(std::vector<std::string>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<std::string>>());
            }
        }
        // std::vector<bool>
        if (expected_type == std::type_index(typeid(std::vector<bool>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::vector<bool>>());
            }
        }

        // Handle std::map<K,V> conversions from Python dicts
        if (expected_type == std::type_index(typeid(std::map<std::string, int>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<std::string, int>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::map<std::string, double>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<std::string, double>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::map<std::string, std::string>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<std::string, std::string>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::map<int, int>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<int, int>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::map<int, double>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<int, double>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::map<int, std::string>))) {
            if (pyb11::isinstance<pyb11::dict>(py_obj)) {
                return core::Any(py_obj.cast<std::map<int, std::string>>());
            }
        }

        // Handle std::set<T> conversions from Python sets or lists
        if (expected_type == std::type_index(typeid(std::set<int>))) {
            if (pyb11::isinstance<pyb11::set>(py_obj) || pyb11::isinstance<pyb11::list>(py_obj)) {
                return core::Any(py_obj.cast<std::set<int>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::set<double>))) {
            if (pyb11::isinstance<pyb11::set>(py_obj) || pyb11::isinstance<pyb11::list>(py_obj)) {
                return core::Any(py_obj.cast<std::set<double>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::set<std::string>))) {
            if (pyb11::isinstance<pyb11::set>(py_obj) || pyb11::isinstance<pyb11::list>(py_obj)) {
                return core::Any(py_obj.cast<std::set<std::string>>());
            }
        }

        // Handle std::array<T, N> conversions from Python lists/tuples
        if (expected_type == std::type_index(typeid(std::array<double, 2>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<double, 2>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::array<double, 3>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<double, 3>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::array<double, 4>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<double, 4>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::array<int, 2>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<int, 2>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::array<int, 3>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<int, 3>>());
            }
        }
        if (expected_type == std::type_index(typeid(std::array<int, 4>))) {
            if (pyb11::isinstance<pyb11::list>(py_obj) || pyb11::isinstance<pyb11::tuple>(py_obj)) {
                return core::Any(py_obj.cast<std::array<int, 4>>());
            }
        }

        // Handle std::function types - add this BEFORE the final return statement
        if (pyb11::isinstance<pyb11::function>(py_obj) || pyb11::hasattr(py_obj, "__call__")) {
            auto &registry = TypeConverterRegistry::instance();
            if (registry.has_converter(expected_type)) {
                return registry.convert(py_obj, expected_type);
            }
        }

        // Try custom type converters for registered classes (including vector types)
        auto &registry = TypeConverterRegistry::instance();
        if (registry.has_converter(expected_type)) {
            return registry.convert(py_obj, expected_type);
        }

        return core::Any(); // Empty
    }

    /**
     * @brief Wrapper to convert Python callable to std::function
     * This allows Python lambdas to be used where C++ expects std::function
     */
    template <typename Ret, typename... Args>
    std::function<Ret(Args...)> python_callable_to_function(const pyb11::object &py_callable) {
        // Capture the Python callable in a shared_ptr to manage its lifetime
        auto py_func_ptr = std::make_shared<pyb11::object>(py_callable);

        return [py_func_ptr](Args... args) -> Ret {
            // Acquire GIL for calling Python from C++
            pyb11::gil_scoped_acquire acquire;

            try {
                // Call the Python function with converted arguments
                pyb11::object result = (*py_func_ptr)(args...);

                if constexpr (std::is_void_v<Ret>) {
                    return;
                } else {
                    // Convert result back to C++ type
                    return result.cast<Ret>();
                }
            } catch (const pyb11::error_already_set &e) {
                throw std::runtime_error(std::string("Python callable failed: ") + e.what());
            }
        };
    }

    /**
     * @brief Helper to register std::function converters
     * Must be called before binding classes that use std::function
     */
    template <typename Ret, typename... Args> void register_function_converter() {
        using FuncType                  = std::function<Ret(Args...)>;
        std::type_index func_type_index = std::type_index(typeid(FuncType));

        // Create converter function
        auto converter = [](const pyb11::object &obj) -> core::Any {
            if (pyb11::isinstance<pyb11::function>(obj) || pyb11::hasattr(obj, "__call__")) {
                FuncType cpp_func = python_callable_to_function<Ret, Args...>(obj);
                return core::Any(cpp_func);
            }
            throw std::runtime_error("Object is not callable");
        };

        // Register using the public API
        TypeConverterRegistry::instance().register_custom_converter(func_type_index, converter);
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
        static void bind(pyb11::module_ &m, const std::string &py_name = "") {
            // Get metadata from registry
            const auto &meta       = core::Registry::instance().get<T>();
            std::string final_name = py_name.empty() ? meta.name() : py_name;

            // Register type converter for this class (Python -> C++)
            TypeConverterRegistry::instance().register_converter<T>();

            // Register type cast function for this class (C++ -> Python)
            TypeCastRegistry::instance().register_cast<T>();

            // Automatically register std::vector<T> converters
            TypeConverterRegistry::instance().register_converter<std::vector<T>>();
            TypeCastRegistry::instance().register_cast<std::vector<T>>();

            // Create the pybind11 class
            pyb11::class_<T> py_class(m, final_name.c_str());

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
        static void bind_constructors(pyb11::class_<T>             &py_class,
                                      const core::ClassMetadata<T> &meta) {
            // Check if we have constructor_infos() method (new API)
            if constexpr (requires { meta.constructor_infos(); }) {
                const auto &ctor_infos = meta.constructor_infos();

                if (ctor_infos.empty()) {
                    // No registered constructors, try default
                    if constexpr (std::is_default_constructible_v<T>) {
                        py_class.def(pyb11::init<>());
                    }
                    return;
                }

                // Bind each constructor with known parameter types
                for (const auto &ctor_info : ctor_infos) {
                    if (ctor_info.arity == 0) {
                        // Default constructor
                        py_class.def(pyb11::init([ctor = ctor_info.invoker]() {
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
                        py_class.def(pyb11::init<>());
                    }
                    return;
                }

                for (const auto &ctor : ctors) {
                    int arity = detect_constructor_arity(ctor);
                    if (arity == 0) {
                        py_class.def(pyb11::init([ctor]() {
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
        static void bind_typed_constructor(pyb11::class_<T> &py_class, const CtorInfo &ctor_info) {
            py_class.def(pyb11::init([ctor = ctor_info.invoker, param_types = ctor_info.param_types,
                                      arity = ctor_info.arity](pyb11::args args) {
                if (args.size() != arity) {
                    throw std::runtime_error("Constructor expects " + std::to_string(arity) +
                                             " arguments, got " + std::to_string(args.size()));
                }

                // Convert Python args to C++ Any using the actual parameter types
                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    pyb11::object arg = args[i];
                    // Use python_to_any with the expected type for proper conversion
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
         * Fallback when we don't have type information - tries to infer types
         */
        static void
        bind_parametric_constructor(pyb11::class_<T> &py_class,
                                    const std::function<core::Any(std::vector<core::Any>)> &ctor,
                                    int                                                     arity) {

            // Create a lambda that accepts py::args
            py_class.def(pyb11::init([ctor, arity](pyb11::args args) {
                if (static_cast<int>(args.size()) != arity) {
                    throw std::runtime_error("Constructor expects " + std::to_string(arity) +
                                             " arguments, got " + std::to_string(args.size()));
                }

                // Convert Python args to Any - try to infer the type
                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < args.size(); ++i) {
                    pyb11::object arg = args[i];

                    // Try to infer type from Python object
                    std::type_index inferred_type = std::type_index(typeid(void));

                    if (pyb11::isinstance<pyb11::int_>(arg)) {
                        inferred_type = std::type_index(typeid(int));
                    } else if (pyb11::isinstance<pyb11::float_>(arg)) {
                        inferred_type = std::type_index(typeid(double));
                    } else if (pyb11::isinstance<pyb11::bool_>(arg)) {
                        inferred_type = std::type_index(typeid(bool));
                    } else if (pyb11::isinstance<pyb11::str>(arg)) {
                        inferred_type = std::type_index(typeid(std::string));
                    } else if (pyb11::isinstance<pyb11::list>(arg) ||
                               pyb11::isinstance<pyb11::tuple>(arg)) {
                        // For lists, try to infer element type from first element if possible
                        pyb11::list py_list = arg.cast<pyb11::list>();
                        if (py_list.size() > 0) {
                            pyb11::object first = py_list[0];
                            if (pyb11::isinstance<pyb11::int_>(first)) {
                                inferred_type = std::type_index(typeid(std::vector<int>));
                            } else if (pyb11::isinstance<pyb11::float_>(first)) {
                                inferred_type = std::type_index(typeid(std::vector<double>));
                            } else if (pyb11::isinstance<pyb11::str>(first)) {
                                inferred_type = std::type_index(typeid(std::vector<std::string>));
                            }
                        }
                    }

                    // Use python_to_any with inferred type
                    if (inferred_type != std::type_index(typeid(void))) {
                        cpp_args.push_back(python_to_any(arg, inferred_type));
                    } else {
                        // Fallback: try basic conversion
                        if (pyb11::isinstance<pyb11::int_>(arg)) {
                            cpp_args.emplace_back(arg.cast<int>());
                        } else if (pyb11::isinstance<pyb11::float_>(arg)) {
                            cpp_args.emplace_back(arg.cast<double>());
                        } else if (pyb11::isinstance<pyb11::bool_>(arg)) {
                            cpp_args.emplace_back(arg.cast<bool>());
                        } else if (pyb11::isinstance<pyb11::str>(arg)) {
                            cpp_args.emplace_back(arg.cast<std::string>());
                        } else {
                            throw std::runtime_error("Unsupported argument type at position " +
                                                     std::to_string(i));
                        }
                    }
                }

                core::Any result = ctor(cpp_args);
                return result.as<T>();
            }));
        }

        /**
         * @brief Bind all fields as properties
         */
        static void bind_fields(pyb11::class_<T> &py_class, const core::ClassMetadata<T> &meta) {
            const auto &fields = meta.fields();

            for (const auto &field_name : fields) {
                std::type_index field_type = meta.get_field_type(field_name);

                // Create getter
                auto getter = [field_name, &meta](T &obj) -> pyb11::object {
                    try {
                        core::Any value = meta.get_field(obj, field_name);
                        return any_to_python(value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error(std::string("Failed to get field '") + field_name +
                                                 "': " + e.what());
                    }
                };

                // Create setter
                auto setter = [field_name, field_type, &meta](T &obj, pyb11::object value) {
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
        static void bind_methods(pyb11::class_<T> &py_class, const core::ClassMetadata<T> &meta) {
            const auto &methods = meta.methods();

            for (const auto &method_name : methods) {
                size_t          arity       = meta.get_method_arity(method_name);
                const auto     &arg_types   = meta.get_method_arg_types(method_name);
                std::type_index return_type = meta.get_method_return_type(method_name);

                // Create method wrapper that accepts py::args
                auto method_wrapper = [method_name, arity, arg_types, return_type,
                                       &meta](T &obj, pyb11::args args) -> pyb11::object {
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

    inline PyGenerator::PyGenerator(pyb11::module_ &m) : module_(m) {
    }

    template <typename T> inline PyGenerator &PyGenerator::bind_class(const std::string &py_name) {
        PyClassBinder<T>::bind(module_, py_name);
        return *this;
    }

    template <typename Ret, typename... Args>
    inline PyGenerator &PyGenerator::bind_function(const std::string &name, Ret (*func)(Args...),
                                                   const std::string &docstring) {
        // Use pybind11's automatic function binding
        module_.def(name.c_str(), func, docstring.c_str());
        return *this;
    }

    inline PyGenerator &PyGenerator::add_utilities() {
        // List all registered classes
        module_.def(
            "list_classes", []() { return core::Registry::instance().list_classes(); },
            "List all registered classes");

        // Get class information
        module_.def(
            "get_class_info",
            [](const std::string &name) -> pyb11::dict {
                auto holder = core::Registry::instance().get_by_name(name);
                if (!holder) {
                    throw std::runtime_error("Class not found: " + name);
                }

                pyb11::dict info;
                info["name"] = holder->get_name();

                const auto &inh                = holder->get_inheritance();
                info["is_abstract"]            = inh.is_abstract;
                info["is_polymorphic"]         = inh.is_polymorphic;
                info["has_virtual_destructor"] = inh.has_virtual_destructor;
                info["base_count"]             = inh.total_base_count();

                return info;
            },
            pyb11::arg("name"), "Get information about a registered class");

        // Version info
        module_.def("version", []() { return rosetta::version(); }, "Get Rosetta version");

        return *this;
    }

    inline PyGenerator &PyGenerator::set_doc(const std::string &doc) {
        module_.doc() = doc.c_str();
        return *this;
    }

    /**
     * @brief Bind multiple classes at once
     */
    template <typename... Classes> inline void bind_classes(PyGenerator &gen) {
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
    inline PyGenerator create_bindings(pyb11::module_ &m) {
        return PyGenerator(m);
    }

} // namespace rosetta::py