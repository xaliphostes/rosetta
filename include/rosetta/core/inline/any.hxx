namespace rosetta::core {

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
        // Strip cv-qualifiers and references to get the base type
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;

        if (!holder_) {
            throw std::bad_cast();
        }

        std::type_index actual_type   = holder_->get_type_index();
        std::type_index expected_type = std::type_index(typeid(BaseType));

        // Direct type match - fast path
        if (actual_type == expected_type) {
            return static_cast<const HolderImpl<BaseType> *>(holder_.get())->value;
        }

        // Numeric conversions: allow double <-> int/float conversions
        if constexpr (std::is_arithmetic_v<BaseType>) {
            // Try to convert from double
            if (actual_type == std::type_index(typeid(double))) {
                double val = static_cast<const HolderImpl<double> *>(holder_.get())->value;
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
            // Try to convert from int
            if (actual_type == std::type_index(typeid(int))) {
                int val = static_cast<const HolderImpl<int> *>(holder_.get())->value;
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
            // Try to convert from float
            if (actual_type == std::type_index(typeid(float))) {
                float val = static_cast<const HolderImpl<float> *>(holder_.get())->value;
                static thread_local BaseType converted;
                converted = static_cast<BaseType>(val);
                return converted;
            }
        }

        // Type mismatch
        throw std::bad_cast();
    }

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
