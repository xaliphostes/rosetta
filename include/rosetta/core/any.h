// ============================================================================
// rosetta/core/any.hpp
//
// Type erasure to safely store any type in Rosetta
// ============================================================================
#pragma once
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace rosetta::core {

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
         * @brief Constructor with value
         * @tparam T Type of the value
         * @param value Value to store
         */
        template <typename T> Any(T value) : holder_(new HolderImpl<T>(std::move(value))) {}

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
         * @brief Get the stored value
         * @tparam T Expected type
         * @return Reference to the stored value
         * @throws std::bad_cast if the type does not match
         */
        template <typename T> T &as() { 
            if (!holder_) {
                throw std::bad_cast();
            }
            
            std::type_index actual_type = holder_->get_type_index();
            std::type_index expected_type = std::type_index(typeid(T));
            
            // Direct type match - fast path
            if (actual_type == expected_type) {
                return static_cast<HolderImpl<T> *>(holder_.get())->value;
            }
            
            // Numeric conversions: allow double <-> int/float conversions
            if constexpr (std::is_arithmetic_v<T>) {
                // Try to convert from double
                if (actual_type == std::type_index(typeid(double))) {
                    double val = static_cast<HolderImpl<double> *>(holder_.get())->value;
                    // Store converted value temporarily - this is a workaround
                    // Create a new Any with the converted value and return its reference
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
                // Try to convert from int
                if (actual_type == std::type_index(typeid(int))) {
                    int val = static_cast<HolderImpl<int> *>(holder_.get())->value;
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
                // Try to convert from float
                if (actual_type == std::type_index(typeid(float))) {
                    float val = static_cast<HolderImpl<float> *>(holder_.get())->value;
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
            }
            
            // Type mismatch
            throw std::bad_cast();
        }

        /**
         * @brief Get the stored value (const version)
         */
        template <typename T> const T &as() const {
            if (!holder_) {
                throw std::bad_cast();
            }
            
            std::type_index actual_type = holder_->get_type_index();
            std::type_index expected_type = std::type_index(typeid(T));
            
            // Direct type match - fast path
            if (actual_type == expected_type) {
                return static_cast<const HolderImpl<T> *>(holder_.get())->value;
            }
            
            // Numeric conversions: allow double <-> int/float conversions
            if constexpr (std::is_arithmetic_v<T>) {
                // Try to convert from double
                if (actual_type == std::type_index(typeid(double))) {
                    double val = static_cast<const HolderImpl<double> *>(holder_.get())->value;
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
                // Try to convert from int
                if (actual_type == std::type_index(typeid(int))) {
                    int val = static_cast<const HolderImpl<int> *>(holder_.get())->value;
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
                // Try to convert from float
                if (actual_type == std::type_index(typeid(float))) {
                    float val = static_cast<const HolderImpl<float> *>(holder_.get())->value;
                    static thread_local T converted;
                    converted = static_cast<T>(val);
                    return converted;
                }
            }
            
            // Type mismatch
            throw std::bad_cast();
        }

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

        std::type_index get_type_index() const {
            if (!holder_) {
                return std::type_index(typeid(void));
            }
            return holder_->get_type_index();
        }

        const void *get_void_ptr() const {
            if (!holder_)
                return nullptr;
            return holder_->get_void_ptr();
        }
    };

} // namespace rosetta::core