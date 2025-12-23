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
                    // For abstract classes or polymorphic types, work with pointers
                    if constexpr (std::is_abstract_v<T> || std::is_polymorphic_v<T>) {
                        T *cpp_ptr = obj.cast<T *>();
                        return core::Any(cpp_ptr);
                    } else {
                        // Try to cast the Python object to the C++ type
                        // pybind11 will handle the conversion for registered types
                        T cpp_obj = obj.cast<T>();
                        return core::Any(cpp_obj);
                    }
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
                    if constexpr (std::is_abstract_v<T> || std::is_polymorphic_v<T>) {
                        T *obj = value.as<T *>();
                        return pyb11::cast(obj);
                    } else {
                        T obj = value.as<T>();
                        return pyb11::cast(obj);
                    }
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Failed to cast C++ object to Python: ") +
                                             e.what());
                }
            };
        }

        /**
         * @brief Register a custom cast function for a specific type
         * @param type_index Type to register cast for
         * @param cast_func Cast function
         */
        void register_custom_cast(std::type_index type_index, CastFunc cast_func) {
            cast_funcs_[type_index] = std::move(cast_func);
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

    // ============================================================================
    // Generic Container Type Binding Functions
    // ============================================================================

    /**
     * @brief Register converters for std::vector<T>
     * @tparam T Element type
     *
     * This allows automatic conversion between Python lists and std::vector<T>
     *
     * Example usage:
     * ```cpp
     * bind_vector_type<MyClass>();
     * bind_vector_type<std::string>();
     * ```
     */
    template <typename T> inline void bind_vector_type() {
        // Register converter: Python list -> std::vector<T>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::vector<T>)), [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::list>(obj) || pyb11::isinstance<pyb11::tuple>(obj)) {
                    try {
                        return core::Any(obj.cast<std::vector<T>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python list to std::vector: ") +
                            e.what());
                    }
                }
                throw std::runtime_error("Expected a Python list or tuple");
            });

        // Register cast: std::vector<T> -> Python list
        TypeCastRegistry::instance().register_cast<std::vector<T>>();
    }

    /**
     * @brief Register converters for std::map<K,V>
     * @tparam K Key type
     * @tparam V Value type
     *
     * This allows automatic conversion between Python dicts and std::map<K,V>
     *
     * Example usage:
     * ```cpp
     * bind_map_type<std::string, int>();
     * bind_map_type<int, MyClass>();
     * ```
     */
    template <typename K, typename V> inline void bind_map_type() {
        // Register converter: Python dict -> std::map<K,V>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::map<K, V>)), [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::dict>(obj)) {
                    try {
                        return core::Any(obj.cast<std::map<K, V>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python dict to std::map: ") + e.what());
                    }
                }
                throw std::runtime_error("Expected a Python dict");
            });

        // Register cast: std::map<K,V> -> Python dict
        TypeCastRegistry::instance().register_cast<std::map<K, V>>();
    }

    /**
     * @brief Register converters for std::set<T>
     * @tparam T Element type
     *
     * This allows automatic conversion between Python sets and std::set<T>
     *
     * Example usage:
     * ```cpp
     * bind_set_type<int>();
     * bind_set_type<std::string>();
     * ```
     */
    template <typename T> inline void bind_set_type() {
        // Register converter: Python set -> std::set<T>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::set<T>)), [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::set>(obj) || pyb11::isinstance<pyb11::list>(obj)) {
                    try {
                        return core::Any(obj.cast<std::set<T>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python set to std::set: ") + e.what());
                    }
                }
                throw std::runtime_error("Expected a Python set or list");
            });

        // Register cast: std::set<T> -> Python set
        TypeCastRegistry::instance().register_cast<std::set<T>>();
    }

    /**
     * @brief Register converters for std::array<T,N>
     * @tparam T Element type
     * @tparam N Array size
     *
     * This allows automatic conversion between Python lists/tuples and std::array<T,N>
     *
     * Example usage:
     * ```cpp
     * bind_array_type<double, 3>();  // For 3D vectors
     * bind_array_type<int, 4>();     // For RGBA colors
     * ```
     */
    template <typename T, size_t N> inline void bind_array_type() {
        // Register converter: Python list/tuple -> std::array<T,N>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::array<T, N>)), [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::list>(obj) || pyb11::isinstance<pyb11::tuple>(obj)) {
                    try {
                        auto arr = obj.cast<std::array<T, N>>();
                        return core::Any(arr);
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python list to std::array: ") +
                            e.what());
                    }
                }
                throw std::runtime_error("Expected a Python list or tuple");
            });

        // Register cast: std::array<T,N> -> Python list
        TypeCastRegistry::instance().register_cast<std::array<T, N>>();
    }

    /**
     * @brief Register converters for std::unordered_map<K,V>
     * @tparam K Key type
     * @tparam V Value type
     *
     * Example usage:
     * ```cpp
     * bind_unordered_map_type<std::string, int>();
     * ```
     */
    template <typename K, typename V> inline void bind_unordered_map_type() {
        // Register converter: Python dict -> std::unordered_map<K,V>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::unordered_map<K, V>)),
            [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::dict>(obj)) {
                    try {
                        return core::Any(obj.cast<std::unordered_map<K, V>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python dict to std::unordered_map: ") +
                            e.what());
                    }
                }
                throw std::runtime_error("Expected a Python dict");
            });

        // Register cast: std::unordered_map<K,V> -> Python dict
        TypeCastRegistry::instance().register_cast<std::unordered_map<K, V>>();
    }

    /**
     * @brief Register converters for std::unordered_set<T>
     * @tparam T Element type
     *
     * Example usage:
     * ```cpp
     * bind_unordered_set_type<std::string>();
     * ```
     */
    template <typename T> inline void bind_unordered_set_type() {
        // Register converter: Python set -> std::unordered_set<T>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::unordered_set<T>)),
            [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::set>(obj) || pyb11::isinstance<pyb11::list>(obj)) {
                    try {
                        return core::Any(obj.cast<std::unordered_set<T>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python set to std::unordered_set: ") +
                            e.what());
                    }
                }
                throw std::runtime_error("Expected a Python set or list");
            });

        // Register cast: std::unordered_set<T> -> Python set
        TypeCastRegistry::instance().register_cast<std::unordered_set<T>>();
    }

    /**
     * @brief Register converters for std::deque<T>
     * @tparam T Element type
     *
     * Example usage:
     * ```cpp
     * bind_deque_type<int>();
     * ```
     */
    template <typename T> inline void bind_deque_type() {
        // Register converter: Python list -> std::deque<T>
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::deque<T>)), [](const pyb11::object &obj) -> core::Any {
                if (pyb11::isinstance<pyb11::list>(obj)) {
                    try {
                        return core::Any(obj.cast<std::deque<T>>());
                    } catch (const pyb11::cast_error &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert Python list to std::deque: ") +
                            e.what());
                    }
                }
                throw std::runtime_error("Expected a Python list");
            });

        // Register cast: std::deque<T> -> Python list
        TypeCastRegistry::instance().register_cast<std::deque<T>>();
    }

    template <typename T> inline void bind_shared_ptr_arg_type() {
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(std::shared_ptr<T>)), [](const pyb11::object &obj) -> core::Any {
                T                 *raw_ptr = obj.cast<T *>();
                std::shared_ptr<T> shared_ptr(raw_ptr, [](T *) {}); // Non-owning
                return core::Any(shared_ptr);
            });
        TypeCastRegistry::instance().register_cast<std::shared_ptr<T>>();
    }

    /**
     * @brief Register converters for raw pointer argument types T*
     * This allows passing Python objects where C++ expects T*
     *
     * Example usage:
     * ```cpp
     * bind_ptr_arg_type<Model>();  // Allows Model* in constructors/methods
     * ```
     */
    template <typename T> inline void bind_ptr_arg_type() {
        // Register converter: Python object -> T*
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(T *)), [](const pyb11::object &obj) -> core::Any {
                try {
                    T *raw_ptr = obj.cast<T *>();
                    return core::Any(raw_ptr);
                } catch (const pyb11::cast_error &e) {
                    throw std::runtime_error(
                        std::string("Failed to convert Python object to pointer: ") + e.what());
                }
            });

        // Also register the cast back (C++ T* -> Python)
        TypeCastRegistry::instance().register_custom_cast(
            std::type_index(typeid(T *)), [](const core::Any &value) -> pyb11::object {
                try {
                    T *ptr = value.as<T *>();
                    return pyb11::cast(ptr);
                } catch (const std::exception &e) {
                    throw std::runtime_error(std::string("Failed to cast pointer to Python: ") +
                                             e.what());
                }
            });
    }

    /**
     * @brief Register converters for reference argument types T&
     * This allows passing Python objects where C++ expects T&
     * by extracting a pointer and dereferencing
     *
     * Note: References are stored as pointers in Any and dereferenced during invocation
     *
     * Example usage:
     * ```cpp
     * bind_ref_arg_type<Model>();  // Allows Model& in constructors/methods
     * ```
     */
    template <typename T> inline void bind_ref_arg_type() {
        // For references, we store as pointer in Any
        // The invoker will need to handle dereferencing
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(T &)), [](const pyb11::object &obj) -> core::Any {
                try {
                    T *raw_ptr = obj.cast<T *>();
                    return core::Any(raw_ptr); // Store as pointer
                } catch (const pyb11::cast_error &e) {
                    throw std::runtime_error(
                        std::string("Failed to convert Python object to reference: ") + e.what());
                }
            });
    }

    // =========================================================================
    // Direct Constructor Binding Helpers (bypass Rosetta's type-erased invoker)
    // Use these for constructors with reference parameters
    // =========================================================================

    /**
     * @brief Directly bind a constructor taking a single reference parameter
     * This bypasses Rosetta's type-erased invoker to properly handle references
     */
    template <typename T, typename Arg1>
    void bind_constructor_1ref(pyb11::class_<T, std::shared_ptr<T>> &py_class) {
        py_class.def(pyb11::init([](Arg1 &arg1) { return T(arg1); }));
    }

    /**
     * @brief Directly bind a constructor taking two parameters (first is reference)
     */
    template <typename T, typename Arg1, typename Arg2>
    void bind_constructor_2args_ref1(pyb11::class_<T, std::shared_ptr<T>> &py_class) {
        py_class.def(pyb11::init([](Arg1 &arg1, Arg2 arg2) { return T(arg1, arg2); }));
    }

    // =========================================================================
    // Helper to get pybind11 class from module (for manual constructor binding)
    // =========================================================================

    template <typename T>
    pyb11::class_<T, std::shared_ptr<T>> &get_py_class(pyb11::module_ &m, const std::string &name) {
        // This is a workaround - in practice, store the class during bind_class
        static std::unordered_map<std::string,
                                  std::unique_ptr<pyb11::class_<T, std::shared_ptr<T>>>>
            classes;
        // Note: This approach has limitations; see BIND_CLASS_WITH_REF_CTOR below
        return *classes[name];
    }

    // =========================================================================
    // Helper to bind base class methods to a derived class
    // =========================================================================

    /**
     * @brief Bind methods from a base class to a derived class pybind11 binding
     * This is needed when using BIND_DERIVED_CLASS_WITH_REF_CTOR macros
     * because they bypass the normal Rosetta binding which handles inheritance
     */
    template <typename Derived, typename Base, typename PyClass>
    void bind_base_methods(PyClass &py_class) {
        try {
            const auto &base_meta = core::Registry::instance().get<Base>();
            const auto &methods   = base_meta.methods();

            for (const auto &method_name : methods) {
                try {
                    size_t          arity       = base_meta.get_method_arity(method_name);
                    const auto     &arg_types   = base_meta.get_method_arg_types(method_name);
                    std::type_index return_type = base_meta.get_method_return_type(method_name);

                    // Create method wrapper
                    auto method_wrapper = [method_name, arity, arg_types, &base_meta](
                                              Derived &obj, pyb11::args args) -> pyb11::object {
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

                        // Invoke the method on the base class reference
                        try {
                            Base     &base_ref = static_cast<Base &>(obj);
                            core::Any result =
                                base_meta.invoke_method(base_ref, method_name, cpp_args);
                            return any_to_python(result);
                        } catch (const std::exception &e) {
                            throw std::runtime_error(std::string("Method '") + method_name +
                                                     "' failed: " + e.what());
                        }
                    };

                    // Register the method
                    py_class.def(method_name.c_str(), method_wrapper);
                } catch (const std::out_of_range &) {
                    // Skip methods without implementation info
                    continue;
                }
            }
        } catch (const std::exception &e) {
            // Base class not registered, skip
        }
    }

    // ========================================================================

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
    template <typename Ret, typename... Args> inline void register_function_converter() {
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

            // Automatically register vector converters
            // For abstract/polymorphic classes, use std::vector<std::shared_ptr<T>>
            // For concrete classes, use std::vector<T>
            if constexpr (std::is_abstract_v<T> || std::is_polymorphic_v<T>) {
                TypeConverterRegistry::instance()
                    .register_converter<std::vector<std::shared_ptr<T>>>();
                TypeCastRegistry::instance().register_cast<std::vector<std::shared_ptr<T>>>();
            } else {
                TypeConverterRegistry::instance().register_converter<std::vector<T>>();
                TypeCastRegistry::instance().register_cast<std::vector<T>>();
            }

            // Get inheritance information
            // const auto &inh = meta.inheritance();

            // Create the pybind11 class with appropriate holder type and base class if needed
            // For abstract or polymorphic classes, use std::shared_ptr as holder
            pyb11::class_<T, std::shared_ptr<T>> py_class(m, final_name.c_str());

            // Note: pybind11 base class declaration would need to be done at compile-time
            // with a template parameter like: pyb11::class_<Derived, Base,
            // std::shared_ptr<Derived>> Since we're doing runtime binding, we can't automatically
            // add base classes here Users should manually declare inheritance when calling
            // BIND_PY_CLASS

            // Bind constructors (abstract classes won't have constructors)
            if constexpr (!std::is_abstract_v<T>) {
                bind_constructors(py_class, meta);
            }

            // Bind fields
            bind_fields(py_class, meta);

            // Bind methods
            bind_methods(py_class, meta);

            // Add __repr__ for better debugging
            py_class.def("__repr__",
                         [final_name](const T &obj) { return "<" + final_name + " object>"; });
        }

        /**
         * @brief Bind only fields and methods (no constructors)
         * Used by BIND_CLASS_WITH_REF_CTOR macros
         */
        static void bind_methods_only(pyb11::class_<T, std::shared_ptr<T>> &py_class,
                                      const core::ClassMetadata<T>         &meta) {
            // Register type converter for this class (Python -> C++)
            TypeConverterRegistry::instance().register_converter<T>();

            // Register type cast function for this class (C++ -> Python)
            TypeCastRegistry::instance().register_cast<T>();

            // Automatically register vector converters
            if constexpr (std::is_abstract_v<T> || std::is_polymorphic_v<T>) {
                TypeConverterRegistry::instance()
                    .register_converter<std::vector<std::shared_ptr<T>>>();
                TypeCastRegistry::instance().register_cast<std::vector<std::shared_ptr<T>>>();
            } else {
                TypeConverterRegistry::instance().register_converter<std::vector<T>>();
                TypeCastRegistry::instance().register_cast<std::vector<T>>();
            }

            // Bind fields
            bind_fields(py_class, meta);

            // Bind methods
            bind_methods(py_class, meta);
        }

        /**
         * @brief Bind all constructors
         */
        static void bind_constructors(pyb11::class_<T, std::shared_ptr<T>> &py_class,
                                      const core::ClassMetadata<T>         &meta) {
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
        static void bind_typed_constructor(pyb11::class_<T, std::shared_ptr<T>> &py_class,
                                           const CtorInfo                       &ctor_info) {
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
        bind_parametric_constructor(pyb11::class_<T, std::shared_ptr<T>> &py_class,
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
        static void bind_fields(pyb11::class_<T, std::shared_ptr<T>> &py_class,
                                const core::ClassMetadata<T>         &meta) {
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
        static void bind_methods(pyb11::class_<T, std::shared_ptr<T>> &py_class,
                                 const core::ClassMetadata<T>         &meta) {
            const auto &methods = meta.methods();

            for (const auto &method_name : methods) {
                // Check if this method has method_info (pure virtual methods won't have it)
                // Pure virtual methods are registered in method_names_ but not in method_info_
                try {
                    size_t          arity       = meta.get_method_arity(method_name);
                    const auto     &arg_types   = meta.get_method_arg_types(method_name);
                    std::type_index return_type = meta.get_method_return_type(method_name);

                    // Create method wrapper that accepts py::args
                    auto method_wrapper = [method_name, arity, arg_types,
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
                } catch (const std::out_of_range &) {
                    // This is a pure virtual method with no implementation info
                    // Skip it - it will be bound in derived classes that implement it
                    continue;
                }
            }
        }
    };

    // ============================================================================
    // Python Derived Class Binding Helper (with Base Class)
    // ============================================================================

    /**
     * @brief Helper to bind a derived class with explicit base class
     */
    template <typename Derived, typename Base> class PyDerivedClassBinder {
    public:
        /**
         * @brief Bind the derived class to Python module with base class
         */
        static void bind(pyb11::module_ &m, const std::string &py_name = "") {
            // Get metadata from registry
            const auto &meta       = core::Registry::instance().get<Derived>();
            std::string final_name = py_name.empty() ? meta.name() : py_name;

            // Register type converter for this class (Python -> C++)
            TypeConverterRegistry::instance().register_converter<Derived>();

            // Register type cast function for this class (C++ -> Python)
            TypeCastRegistry::instance().register_cast<Derived>();

            // Automatically register vector converters
            // For abstract/polymorphic classes, use std::vector<std::shared_ptr<Derived>>
            // For concrete classes, use std::vector<Derived>
            if constexpr (std::is_abstract_v<Derived> || std::is_polymorphic_v<Derived>) {
                TypeConverterRegistry::instance()
                    .register_converter<std::vector<std::shared_ptr<Derived>>>();
                TypeCastRegistry::instance().register_cast<std::vector<std::shared_ptr<Derived>>>();
            } else {
                TypeConverterRegistry::instance().register_converter<std::vector<Derived>>();
                TypeCastRegistry::instance().register_cast<std::vector<Derived>>();
            }

            // Create the pybind11 class with base class and shared_ptr holder
            pyb11::class_<Derived, Base, std::shared_ptr<Derived>> py_class(m, final_name.c_str());

            // Bind constructors (derived concrete classes will have constructors)
            if constexpr (!std::is_abstract_v<Derived>) {
                PyClassBinder<Derived>::bind_constructors(
                    reinterpret_cast<pyb11::class_<Derived, std::shared_ptr<Derived>> &>(py_class),
                    meta);
            }

            // Bind fields
            PyClassBinder<Derived>::bind_fields(
                reinterpret_cast<pyb11::class_<Derived, std::shared_ptr<Derived>> &>(py_class),
                meta);

            // Bind methods
            PyClassBinder<Derived>::bind_methods(
                reinterpret_cast<pyb11::class_<Derived, std::shared_ptr<Derived>> &>(py_class),
                meta);

            // Add __repr__ for better debugging
            py_class.def("__repr__", [final_name](const Derived &obj) {
                return "<" + final_name + " object>";
            });
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

    template <typename Derived, typename Base>
    inline PyGenerator &PyGenerator::bind_derived_class(const std::string &py_name) {
        PyDerivedClassBinder<Derived, Base>::bind(module_, py_name);
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