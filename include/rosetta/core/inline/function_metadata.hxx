namespace rosetta::core {

    inline FunctionMetadata::FunctionMetadata(std::string name, std::string cpp_name)
        : name_(std::move(name))
        , cpp_name_(cpp_name.empty() ? name_ : std::move(cpp_name))
        , return_type_(typeid(void))
        , arity_(0)
        , is_overloaded_(false) {
    }

    inline const std::string &FunctionMetadata::name() const {
        return name_;
    }

    inline const std::string &FunctionMetadata::cpp_name() const {
        return cpp_name_;
    }

    inline bool FunctionMetadata::is_aliased() const {
        return name_ != cpp_name_;
    }

    inline bool FunctionMetadata::is_overloaded() const {
        return is_overloaded_;
    }

    inline void FunctionMetadata::set_overloaded(bool value) {
        is_overloaded_ = value;
    }

    inline const std::string &FunctionMetadata::func_ptr_type_str() const {
        return func_ptr_type_str_;
    }

    inline void FunctionMetadata::set_func_ptr_type_str(const std::string &type_str) {
        func_ptr_type_str_ = type_str;
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
        
        if (is_aliased()) {
            os << "C++ name: " << cpp_name() << " (aliased)\n";
        }
        if (is_overloaded()) {
            os << "Overloaded: yes\n";
        }

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
        os << "Signature: " << ret_type << " " << cpp_name_ << "(";
        for (size_t i = 0; i < param_types_.size(); ++i) {
            os << get_readable_type_name(param_types_[i]);
            if (i < param_types_.size() - 1) {
                os << ", ";
            }
        }
        os << ")\n";
        os << "================================\n";
    }

    // ========================================================================
    // Extract argument - handles both value and reference parameters
    // Returns reference for non-const lvalue ref params, value otherwise
    // ========================================================================
    template <typename T>
    inline detail::extract_arg_return_t<T> FunctionMetadata::extract_arg(Any &any_val) {
        using RawType = std::remove_cv_t<std::remove_reference_t<T>>;
        
        // Check if we need to return a reference (for non-const lvalue ref params)
        constexpr bool needs_reference = 
            std::is_lvalue_reference_v<T> && 
            !std::is_const_v<std::remove_reference_t<T>>;

        std::type_index actual_type   = any_val.get_type_index();
        std::type_index expected_type = std::type_index(typeid(RawType));

        // Direct match - return reference or value as needed
        if (actual_type == expected_type) {
            // any_val.as<RawType>() returns RawType&
            // For needs_reference=true, we return RawType& (the reference)
            // For needs_reference=false, we return RawType (copy from the reference)
            return any_val.as<RawType>();
        }

        // For non-const lvalue reference parameters, we cannot do type conversions
        // The Any must contain the exact type (or a pointer/reference_wrapper to it)
        if constexpr (needs_reference) {
            // Check if Any contains a reference_wrapper<T>
            std::type_index ref_wrapper_type = std::type_index(typeid(std::reference_wrapper<RawType>));
            if (actual_type == ref_wrapper_type) {
                return any_val.as<std::reference_wrapper<RawType>>().get();
            }
            
            // Check if Any contains a pointer to the type (from Python bindings)
            std::type_index ptr_type = std::type_index(typeid(RawType*));
            if (actual_type == ptr_type) {
                RawType* ptr = any_val.as<RawType*>();
                if (ptr) {
                    return *ptr;
                }
                throw std::runtime_error("Null pointer when non-const reference expected");
            }
            
            // No conversion possible for non-const references - type must match exactly
            throw std::bad_cast();
        }
        
        // For value/const-ref parameters, we can do numeric conversions
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

    // ========================================================================
    // Invoke function with arguments
    // ========================================================================
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

    // ========================================================================
    // Register function
    // ========================================================================
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

} // namespace rosetta::core