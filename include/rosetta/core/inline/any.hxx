#include <rosetta/core/demangler.h>

namespace rosetta::core {

    inline void AnyStringRegistry::register_defaults() {
        register_type<int>([](const int &v) { return std::to_string(v); });
        register_type<float>([](const float &v) { return std::to_string(v); });
        register_type<double>([](const double &v) { return std::to_string(v); });
        register_type<bool>([](const bool &v) { return v ? "true" : "false"; });
        register_type<std::string>([](const std::string &v) { return v; });
    }

    template <typename T>
    inline void AnyStringRegistry::register_type(std::function<std::string(const T &)> fn) {
        converters_[std::type_index(typeid(T))] = [fn](const void *ptr) {
            return fn(*static_cast<const T *>(ptr));
        };
    }

    inline std::string AnyStringRegistry::convert(std::type_index type, const void *ptr) const {
        auto it = converters_.find(type);
        if (it != converters_.end())
            return it->second(ptr);
        return {};
    }

    inline bool AnyStringRegistry::has(std::type_index type) const {
        return converters_.count(type) > 0;
    }

    // ------------------------------------------

    template <typename T> inline Any::Holder *Any::HolderImpl<T>::clone() const {
        if constexpr (std::is_copy_constructible_v<T>) {
            return new HolderImpl(value);
        } else {
            throw std::logic_error("Cannot copy Any containing non-copyable type: " +
                                   std::string(typeid(T).name()));
        }
    }

    template <typename T> inline bool Any::HolderImpl<T>::is_copyable() const {
        return std::is_copy_constructible_v<T>;
    }

    template <typename T> inline std::string Any::HolderImpl<T>::type_name() const {
        return typeid(T).name();
    }

    template <typename T> inline const void *Any::HolderImpl<T>::get_void_ptr() const {
        return &value;
    }

    template <typename T> inline std::type_index Any::HolderImpl<T>::get_type_index() const {
        return std::type_index(typeid(T));
    }

    // ------------------------------------------

    inline std::string Any::toString() const {
        if (!holder_)
            return "<empty>";
        auto &reg  = AnyStringRegistry::instance();
        auto  type = get_type_index();
        if (reg.has(type))
            return reg.convert(type, get_void_ptr());
        return "<" + get_readable_type_name(type) + ">";
    }

    template <typename T>
    inline Any::Any(T value)
        : holder_(new HolderImpl<std::remove_cv_t<std::remove_reference_t<T>>>(std::move(value))) {
    }

    inline Any::Any(const Any &other) : holder_(other.holder_ ? other.holder_->clone() : nullptr) {
    }

    inline Any::Any(const char *str) : holder_(new HolderImpl<std::string>(std::string(str))) {
    }

    inline Any &Any::operator=(const Any &other) {
        if (this != &other) {
            holder_ = other.holder_ ? std::unique_ptr<Holder>(other.holder_->clone()) : nullptr;
        }
        return *this;
    }

    inline std::string Any::type_name() const {
        return holder_ ? holder_->type_name() : "empty";
    }

    inline bool Any::has_value() const {
        return holder_ != nullptr;
    }

    inline bool Any::is_copyable() const {
        return !holder_ || holder_->is_copyable();
    }

    inline void Any::reset() {
        holder_.reset();
    }

    inline std::type_index Any::type() const {
        return get_type_index();
    }

    template <typename T> inline T &Any::as() {
        // Strip cv-qualifiers and references to get the base type
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;

        if (!holder_) {
            throw std::bad_cast();
        }

        std::type_index actual_type   = holder_->get_type_index();
        std::type_index expected_type = std::type_index(typeid(BaseType));

        // Direct type match - fast path
        if (actual_type == expected_type) {
            return static_cast<HolderImpl<BaseType> *>(holder_.get())->value;
        }

        // Case: Stored as reference_wrapper<T> - unwrap it
        if (actual_type == std::type_index(typeid(std::reference_wrapper<BaseType>))) {
            auto *wrapper =
                static_cast<HolderImpl<std::reference_wrapper<BaseType>> *>(holder_.get());
            return wrapper->value.get();
        }

        // Numeric conversions: allow double <-> int/float conversions
        if constexpr (std::is_arithmetic_v<BaseType>) {
            // Try to convert from double
            if (actual_type == std::type_index(typeid(double))) {
                double val = static_cast<HolderImpl<double> *>(holder_.get())->value;
                // Store converted value temporarily - this is a workaround
                // Create a new Any with the converted value and return its reference
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
            // Try to convert from int
            if (actual_type == std::type_index(typeid(int))) {
                int val = static_cast<HolderImpl<int> *>(holder_.get())->value;
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
            // Try to convert from float
            if (actual_type == std::type_index(typeid(float))) {
                float val = static_cast<HolderImpl<float> *>(holder_.get())->value;
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
        }

        // Type mismatch
        throw std::bad_cast();
    }

    template <typename T> inline const T &Any::as() const {
        if (!holder_)
            throw std::bad_cast();

        auto stored_type = holder_->get_type_index();

        // Case 1: Direct match - stored type is exactly T
        if (stored_type == std::type_index(typeid(T))) {
            return *static_cast<const T *>(holder_->get_void_ptr());
        }

        // Case 2: Stored as reference_wrapper<T> - unwrap it
        if (stored_type == std::type_index(typeid(std::reference_wrapper<T>))) {
            const auto *wrapper =
                static_cast<const std::reference_wrapper<T> *>(holder_->get_void_ptr());
            return wrapper->get(); // Returns T& which binds to const T&
        }

        throw std::bad_cast();
    }

    // template <typename T> inline const T &Any::as() const {
    //     // Strip cv-qualifiers and references to get the base type
    //     using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
    //     if (!holder_) {
    //         throw std::bad_cast();
    //     }
    //     std::type_index actual_type   = holder_->get_type_index();
    //     std::type_index expected_type = std::type_index(typeid(BaseType));
    //     // Direct type match - fast path
    //     if (actual_type == expected_type) {
    //         return static_cast<const HolderImpl<BaseType> *>(holder_.get())->value;
    //     }
    //     // Numeric conversions: allow double <-> int/float conversions
    //     if constexpr (std::is_arithmetic_v<BaseType>) {
    //         // Try to convert from double
    //         if (actual_type == std::type_index(typeid(double))) {
    //             double val = static_cast<const HolderImpl<double> *>(holder_.get())->value;
    //             static thread_local BaseType converted;
    //             converted = static_cast<BaseType>(val);
    //             return converted;
    //         }
    //         // Try to convert from int
    //         if (actual_type == std::type_index(typeid(int))) {
    //             int val = static_cast<const HolderImpl<int> *>(holder_.get())->value;
    //             static thread_local BaseType converted;
    //             converted = static_cast<BaseType>(val);
    //             return converted;
    //         }
    //         // Try to convert from float
    //         if (actual_type == std::type_index(typeid(float))) {
    //             float val = static_cast<const HolderImpl<float> *>(holder_.get())->value;
    //             static thread_local BaseType converted;
    //             converted = static_cast<BaseType>(val);
    //             return converted;
    //         }
    //     }
    //     // Type mismatch
    //     throw std::bad_cast();
    // }

    inline std::type_index Any::get_type_index() const {
        if (!holder_) {
            return std::type_index(typeid(void));
        }
        return holder_->get_type_index();
    }

    inline const void *Any::get_void_ptr() const {
        if (!holder_)
            return nullptr;
        return holder_->get_void_ptr();
    }

} // namespace rosetta::core