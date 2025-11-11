namespace rosetta::core {

    inline FunctionMetadata::FunctionMetadata(std::string name)
        : name_(std::move(name)), return_type_(typeid(void)), arity_(0) {
    }

    inline const std::string &FunctionMetadata::name() const {
        return name_;
    }

    inline size_t FunctionMetadata::arity() const {
        return arity_;
    }

    inline const std::vector<std::type_index> &FunctionMetadata::param_types() const {
        return param_types_;
    }

    inline std::type_index FunctionMetadata::return_type() const {
        return return_type_;
    }

    inline Any FunctionMetadata::invoke(std::vector<Any> args) const {
        if (args.size() != arity_) {
            throw std::runtime_error("Function '" + name_ + "' expects " + std::to_string(arity_) +
                                     " arguments, got " + std::to_string(args.size()));
        }
        return invoker_(std::move(args));
    }

    inline void FunctionMetadata::dump(std::ostream &os) const {
        os << "\n=== Function: " << name() << " ===\n";

        // Return type
        std::string ret_type = get_readable_type_name(return_type_);
        os << "Return type: " << ret_type << "\n";

        // Parameters
        os << "Parameters (" << arity_ << "):\n";
        for (size_t i = 0; i < param_types_.size(); ++i) {
            std::string param_type = get_readable_type_name(param_types_[i]);
            os << "  [" << i << "] " << param_type << "\n";
        }

        // Signature
        os << "Signature: " << ret_type << " " << name_ << "(";
        for (size_t i = 0; i < param_types_.size(); ++i) {
            os << get_readable_type_name(param_types_[i]);
            if (i < param_types_.size() - 1) {
                os << ", ";
            }
        }
        os << ")\n";
        os << "================================\n";
    }

    template <typename Ret, typename... Args>
    inline void FunctionMetadata::register_function(Ret (*ptr)(Args...)) {
        // Store type information
        arity_       = sizeof...(Args);
        return_type_ = std::type_index(typeid(Ret));
        param_types_ = {std::type_index(typeid(Args))...};

        // Create invoker
        invoker_ = [ptr](std::vector<Any> args) -> Any {
            if constexpr (sizeof...(Args) == 0) {
                // No arguments
                if constexpr (std::is_void_v<Ret>) {
                    (*ptr)();
                    return Any(0);
                } else {
                    return Any((*ptr)());
                }
            } else {
                // With arguments
                return invoke_with_args(ptr, args, std::index_sequence_for<Args...>{});
            }
        };
    }

    template <typename Ret, typename... Args, size_t... Is>
    inline Any FunctionMetadata::invoke_with_args(Ret (*ptr)(Args...), std::vector<Any> &args,
                                                  std::index_sequence<Is...>) {
        try {
            if constexpr (std::is_void_v<Ret>) {
                (*ptr)(extract_arg<Args>(args[Is])...);
                return Any(0);
            } else {
                return Any((*ptr)(extract_arg<Args>(args[Is])...));
            }
        } catch (const std::bad_cast &e) {
            std::string error = "Type mismatch in function arguments. Expected types: ";
            ((error += typeid(Args).name(), error += " "), ...);
            throw std::runtime_error(error);
        }
    }

    template <typename T>
    inline auto FunctionMetadata::extract_arg(Any &any_val)
        -> std::remove_cv_t<std::remove_reference_t<T>> {
        using RawType = std::remove_cv_t<std::remove_reference_t<T>>;

        std::type_index actual_type   = any_val.get_type_index();
        std::type_index expected_type = std::type_index(typeid(RawType));

        // Direct match
        if (actual_type == expected_type) {
            return any_val.as<RawType>();
        }

        // Numeric conversions
        if constexpr (std::is_arithmetic_v<RawType>) {
            if (actual_type == std::type_index(typeid(double))) {
                return static_cast<RawType>(any_val.as<double>());
            }
            if (actual_type == std::type_index(typeid(int))) {
                return static_cast<RawType>(any_val.as<int>());
            }
            if (actual_type == std::type_index(typeid(float))) {
                return static_cast<RawType>(any_val.as<float>());
            }
        }

        // Fallback to normal extraction
        return any_val.as<RawType>();
    }

} // namespace rosetta::core
