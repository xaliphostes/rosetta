// ============================================================================
// Type erasure to safely store any type in Rosetta
// ============================================================================
#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace rosetta::core {

    /**
     * @brief Registry for converting Any values to human-readable strings.
     *
     * AnyStringRegistry is a singleton that manages type-to-string conversion functions.
     * It allows the `Any::toString()` method to produce meaningful string representations
     * of stored values, rather than just displaying type names.
     *
     * ## Purpose
     *
     * When working with type-erased containers like `Any`, it's often useful to get a
     * human-readable representation of the stored value for debugging, logging, or
     * serialization purposes. Since `Any` erases the type information at compile time,
     * we need a runtime registry to look up the appropriate conversion function.
     *
     * ## Default Registered Types
     *
     * The following types are registered by default:
     * - `int` → uses `std::to_string()`
     * - `float` → uses `std::to_string()`
     * - `double` → uses `std::to_string()`
     * - `bool` → converts to "true" or "false"
     * - `std::string` → returns the string as-is
     *
     * ## Usage Example
     *
     * @code{.cpp}
     * // Register a custom type for string conversion
     * struct Point { double x, y; };
     *
     * AnyStringRegistry::instance().register_type<Point>(
     *     [](const Point& p) {
     *         return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
     *     }
     * );
     *
     * // Now Any::toString() works with Point
     * Any a = Point{3.0, 4.0};
     * std::cout << a.toString();  // Output: "(3.0, 4.0)"
     * @endcode
     *
     * ## Thread Safety
     *
     * The singleton instance is created using the Meyer's singleton pattern,
     * which is thread-safe for initialization in C++11 and later. However,
     * registering new types while other threads are calling `convert()` is
     * not thread-safe. Register all custom types during application startup
     * before any concurrent access.
     *
     * ## Integration with Any
     *
     * This registry is used internally by `Any::toString()`:
     * - If a converter is registered for the stored type, it returns the converted string
     * - If no converter exists, it returns "<TypeName>" with the demangled type name
     * - If the Any is empty, it returns "<empty>"
     *
     * @see Any::toString()
     */
    class AnyStringRegistry {
    public:
        /**
         * @brief Type alias for converter functions.
         *
         * Converters take a void pointer (pointing to the actual value) and return
         * a string representation. The void pointer is cast to the appropriate type
         * inside the type-safe wrapper created by register_type().
         */
        using Converter = std::function<std::string(const void *)>;

        /**
         * @brief Get the singleton instance of the registry.
         *
         * Uses Meyer's singleton pattern for thread-safe lazy initialization.
         *
         * @return Reference to the global AnyStringRegistry instance
         *
         * @code{.cpp}
         * auto& registry = AnyStringRegistry::instance();
         * registry.register_type<MyType>(...);
         * @endcode
         */
        static AnyStringRegistry &instance() {
            static AnyStringRegistry reg;
            return reg;
        }

        /**
         * @brief Register a string converter for a specific type.
         *
         * Registers a function that converts values of type T to their string
         * representation. The function is stored in a type-erased manner using
         * std::type_index as the key.
         *
         * @tparam T The type to register a converter for
         * @param fn A function that takes a const reference to T and returns a string
         *
         * @note If a converter for type T already exists, it will be overwritten.
         *
         * @code{.cpp}
         * // Register converter for a vector
         * AnyStringRegistry::instance().register_type<std::vector<int>>(
         *     [](const std::vector<int>& v) {
         *         std::string result = "[";
         *         for (size_t i = 0; i < v.size(); ++i) {
         *             if (i > 0) result += ", ";
         *             result += std::to_string(v[i]);
         *         }
         *         return result + "]";
         *     }
         * );
         * @endcode
         */
        template <typename T> void register_type(std::function<std::string(const T &)> fn);

        /**
         * @brief Convert a value to its string representation.
         *
         * Looks up the converter for the given type and applies it to the value
         * pointed to by ptr.
         *
         * @param type The type_index of the value to convert
         * @param ptr Pointer to the value (will be cast to the appropriate type)
         * @return The string representation, or empty string if no converter is registered
         *
         * @warning The caller must ensure that ptr actually points to a value of the
         *          type indicated by the type parameter. Passing mismatched types
         *          results in undefined behavior.
         */
        std::string convert(std::type_index type, const void *ptr) const;

        /**
         * @brief Check if a converter is registered for a type.
         *
         * @param type The type_index to check
         * @return true if a converter exists for this type, false otherwise
         *
         * @code{.cpp}
         * if (AnyStringRegistry::instance().has(typeid(MyType))) {
         *     // Converter exists, safe to call convert()
         * }
         * @endcode
         */
        bool has(std::type_index type) const;

    private:
        /**
         * @brief Private constructor - registers default type converters.
         *
         * Called automatically when the singleton is first accessed.
         * Registers converters for: int, float, double, bool, std::string.
         */
        AnyStringRegistry() { register_defaults(); }

        /**
         * @brief Register the default type converters.
         *
         * Called by the constructor to set up converters for common types.
         */
        void register_defaults();

        /**
         * @brief Storage for registered converters, keyed by type_index.
         */
        std::unordered_map<std::type_index, Converter> converters_;
    };

    /**
     * @brief Conteneur type-erased that can store any type.
     * Similar to std::any but with a simplified interface for Rosetta.
     *
     * Supports both copyable and move-only types (e.g., std::unique_ptr).
     * Attempting to copy an Any containing a non-copyable type will throw
     * std::logic_error at runtime.
     */
    class Any {
        struct Holder {
            virtual ~Holder()                              = default;
            virtual Holder         *clone() const          = 0;
            virtual bool            is_copyable() const    = 0;
            virtual std::string     type_name() const      = 0;
            virtual const void     *get_void_ptr() const   = 0;
            virtual std::type_index get_type_index() const = 0;
        };

        template <typename T> struct HolderImpl : Holder {
            T value;
            HolderImpl(T v) : value(std::move(v)) {}
            Holder         *clone() const override;
            bool            is_copyable() const override;
            std::string     type_name() const override;
            const void     *get_void_ptr() const override;
            std::type_index get_type_index() const override;
        };

        std::unique_ptr<Holder> holder_;

    public:
        /**
         * @brief Default constructor (empty Any)
         */
        Any() = default;

        /**
         * @brief Constructor for C-style strings (converts to std::string)
         * This allows {5.0, "hello"} syntax without explicit Any() wrapping
         */
        Any(const char *str);

        /**
         * @brief Constructor with value
         * @tparam T Type of the value
         * @param value Value to store
         */
        template <typename T> Any(T value);

        /**
         * @brief Copy constructor
         * @throws std::logic_error if the stored type is not copyable
         */
        Any(const Any &other);

        /**
         * @brief Move constructor
         */
        Any(Any &&) = default;

        /**
         * @brief Copy assignment operator
         * @throws std::logic_error if the stored type is not copyable
         */
        Any &operator=(const Any &other);

        /**
         * @brief Move assignment operator
         */
        Any &operator=(Any &&) = default;

        /**
         * @brief Give a string representation to this Any
         */
        std::string toString() const;

        /**
         * @brief Get the stored value
         * @tparam T Expected type
         * @return Reference to the stored value
         * @throws std::bad_cast if the type does not match
         */
        template <typename T> T &as();

        /**
         * @brief Get the stored value (const version)
         */
        template <typename T> const T &as() const;

        /**
         * @brief Get the type name of the stored value
         * @return Name of teh type (mangled)
         */
        std::string type_name() const;

        /**
         * @brief Check if the Any contains a value
         * @return true if a value is stored, false otherwise
         */
        bool has_value() const;

        /**
         * @brief Check if the stored type is copyable
         * @return true if the type is copyable or Any is empty
         */
        bool is_copyable() const;

        /**
         * @brief Re-initialize the Any to empty state
         */
        void reset();

        std::type_index get_type_index() const;

        /**
         * @brief Synonymous of get_type_index()
         */
        std::type_index type() const;

        const void *get_void_ptr() const;
    };

} // namespace rosetta::core

#include "inline/any.hxx"