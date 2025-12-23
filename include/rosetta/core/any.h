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

    // For the Any::toString() method
    class AnyStringRegistry {
    public:
        using Converter = std::function<std::string(const void *)>;

        static AnyStringRegistry &instance() {
            static AnyStringRegistry reg;
            return reg;
        }

        template <typename T> void register_type(std::function<std::string(const T &)> fn);
        std::string                convert(std::type_index type, const void *ptr) const;
        bool                       has(std::type_index type) const;

    private:
        AnyStringRegistry() { register_defaults(); }
        void                                           register_defaults();
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