// ============================================================================
// Type erasure to safely store any type in Rosetta
// ============================================================================
#pragma once
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <functional>

namespace rosetta::core {

    // For the Any::toString() method
    class AnyStringRegistry {
    public:
        using Converter = std::function<std::string(const void *)>;

        static AnyStringRegistry &instance() {
            static AnyStringRegistry reg;
            return reg;
        }

        template <typename T> void register_type(std::function<std::string(const T &)> fn) {
            converters_[std::type_index(typeid(T))] = [fn](const void *ptr) {
                return fn(*static_cast<const T *>(ptr));
            };
        }

        std::string convert(std::type_index type, const void *ptr) const {
            auto it = converters_.find(type);
            if (it != converters_.end())
                return it->second(ptr);
            return {};
        }

        bool has(std::type_index type) const { return converters_.count(type) > 0; }

    private:
        AnyStringRegistry() { register_defaults(); }
        void                                           register_defaults();
        std::unordered_map<std::type_index, Converter> converters_;
    };

    /**
     * @brief Conteneur type-erased that can store any type.
     * Similar to std::any but with a simplified interface for Rosetta
     */
    class Any {
        struct Holder {
            virtual ~Holder()                              = default;
            virtual Holder         *clone() const          = 0;
            virtual std::string     type_name() const      = 0;
            virtual const void     *get_void_ptr() const   = 0;
            virtual std::type_index get_type_index() const = 0;
        };

        template <typename T> struct HolderImpl : Holder {
            T value;
            HolderImpl(T v) : value(std::move(v)) {}
            Holder         *clone() const override { return new HolderImpl(value); }
            std::string     type_name() const override { return typeid(T).name(); }
            const void     *get_void_ptr() const override { return &value; }
            std::type_index get_type_index() const override { return std::type_index(typeid(T)); }
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
        Any(const char *str) : holder_(new HolderImpl<std::string>(std::string(str))) {}

        /**
         * @brief Constructor with value
         * @tparam T Type of the value
         * @param value Value to store
         */
        template <typename T>
        Any(T value)
            : holder_(
                  new HolderImpl<std::remove_cv_t<std::remove_reference_t<T>>>(std::move(value))) {}

        /**
         * @brief Copy constructeur
         */
        Any(const Any &other) : holder_(other.holder_ ? other.holder_->clone() : nullptr) {}

        /**
         * @brief Move constructeur.
         */
        Any(Any &&) = default;

        /**
         * @brief Copy assignment operator
         */
        Any &operator=(const Any &other) {
            if (this != &other) {
                holder_ = other.holder_ ? std::unique_ptr<Holder>(other.holder_->clone()) : nullptr;
            }
            return *this;
        }

        /**
         * @brief Move assignment operator
         */
        Any &operator=(Any &&) = default;

        /**
         * @brief Give a string representation to this Any
         */
        std::string toString() const ;

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
        std::string type_name() const { return holder_ ? holder_->type_name() : "empty"; }

        /**
         * @brief Check if the Any contains a value
         * @return true if a value is stored, false otherwise
         */
        bool has_value() const { return holder_ != nullptr; }

        /**
         * @brief Re-initialize the Any to empty state
         */
        void reset() { holder_.reset(); }

        std::type_index get_type_index() const;

        const void *get_void_ptr() const;
    };

} // namespace rosetta::core

#include "inline/any.hxx"
