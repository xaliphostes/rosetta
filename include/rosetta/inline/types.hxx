/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
#include <stdexcept>

namespace rosetta {

    /**
     * @brief Get type name with support for user-registered types
     *
     * This function first checks the TypeNameRegistry for a registered name.
     * If not found, it falls back to built-in type detection.
     * For completely unknown types, it uses typeid(T).name() as last resort.
     */
    template <typename T> inline std::string getTypeName()
    {
        // Remove const, volatile, and reference qualifiers
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;

        // First, check if type is registered in the user registry
        if (TypeNameRegistry::instance().is_registered<BaseType>()) {
            return TypeNameRegistry::instance().get_name<BaseType>();
        }

        // Built-in string types
        if constexpr (std::is_same_v<BaseType, std::string>) {
            return "string";
        } else if constexpr (std::is_same_v<BaseType, char>) {
            return "char";
        } else if constexpr (std::is_same_v<BaseType, unsigned char>) {
            return "unsigned char";
        } else if constexpr (std::is_same_v<BaseType, char*>) {
            return "char*";
        } else if constexpr (std::is_same_v<BaseType, const char*>) {
            return "const char*";
        }
        // Handle pointers to registered types
        else if constexpr (std::is_pointer_v<BaseType>) {
            using PointedType = std::remove_pointer_t<BaseType>;
            if (TypeNameRegistry::instance().is_registered<PointedType>()) {
                return TypeNameRegistry::instance().get_name<PointedType>() + "*";
            }
            return getTypeName<PointedType>() + "*";
        }
        // Handle std::vector of registered types
        else if constexpr (requires {
                               typename BaseType::value_type;
                               typename BaseType::iterator;
                           }) {
            // This is likely a container
            using ValueType = typename BaseType::value_type;
            if (TypeNameRegistry::instance().is_registered<ValueType>()) {
                return "vector<" + TypeNameRegistry::instance().get_name<ValueType>() + ">";
            }
            // Fall through to default handling
        }

        // Unknown type - use mangled name as fallback
        return typeid(BaseType).name();
    }

    // Specialized versions for common built-in types
    template <> inline std::string getTypeName<short>() { return "short"; }

    template <> inline std::string getTypeName<unsigned short>() { return "unsigned short"; }

    template <> inline std::string getTypeName<long>() { return "long"; }

    template <> inline std::string getTypeName<long long>() { return "long long"; }

    template <> inline std::string getTypeName<unsigned int>() { return "unsigned int"; }

    template <> inline std::string getTypeName<size_t>() { return "size_t"; }

    template <> inline std::string getTypeName<int>() { return "int"; }

    template <> inline std::string getTypeName<double>() { return "double"; }

    template <> inline std::string getTypeName<float>() { return "float"; }

    template <> inline std::string getTypeName<bool>() { return "bool"; }

    template <> inline std::string getTypeName<void>() { return "void"; }

    // Specialized versions for common container types
    template <> inline std::string getTypeName<std::vector<int>>() { return "vector<int>"; }

    template <> inline std::string getTypeName<std::vector<float>>() { return "vector<float>"; }

    template <> inline std::string getTypeName<std::vector<double>>() { return "vector<double>"; }

    template <> inline std::string getTypeName<std::vector<std::string>>()
    {
        return "vector<string>";
    }

    // Rest of the file remains the same...

    template <typename Class>
    template <typename MemberType>
    inline TypeRegistrar<Class>& TypeRegistrar<Class>::member(
        const std::string& name, MemberType Class::* member_ptr)
    {
        info.addMember(std::make_unique<MemberInfo>(
            name, getTypeName<MemberType>(),
            [member_ptr](const void* obj) -> std::any {
                const auto* typed_obj = static_cast<const Class*>(obj);
                return std::any { typed_obj->*member_ptr };
            },
            [member_ptr](void* obj, const std::any& value) {
                auto* typed_obj = static_cast<Class*>(obj);
                typed_obj->*member_ptr = std::any_cast<MemberType>(value);
            }));
        return *this;
    }

    // Helper function to create parameter type vector from parameter pack
    template <typename... Args> std::vector<std::string> createParameterTypeVector()
    {
        if constexpr (sizeof...(Args) == 0) {
            return std::vector<std::string> {};
        } else {
            return std::vector<std::string> { getTypeName<Args>()... };
        }
    }

    // Helper function to cast arguments from std::any vector to the correct types
    template <typename Class, typename ReturnType, typename... Args, std::size_t... I>
    inline std::any callMethodImpl(Class* obj, ReturnType (Class::*method_ptr)(Args...),
        const std::vector<std::any>& args, std::index_sequence<I...>)
    {
        if constexpr (std::is_void_v<ReturnType>) {
            (obj->*method_ptr)(std::any_cast<Args>(args[I])...);
            return std::any {};
        } else {
            return std::any { (obj->*method_ptr)(std::any_cast<Args>(args[I])...) };
        }
    }

    // Same for const methods
    template <typename Class, typename ReturnType, typename... Args, std::size_t... I>
    inline std::any callConstMethodImpl(Class* obj, ReturnType (Class::*method_ptr)(Args...) const,
        const std::vector<std::any>& args, std::index_sequence<I...>)
    {
        if constexpr (std::is_void_v<ReturnType>) {
            (obj->*method_ptr)(std::any_cast<Args>(args[I])...);
            return std::any {};
        } else {
            return std::any { (obj->*method_ptr)(std::any_cast<Args>(args[I])...) };
        }
    }

    // Variadic method registration for non-const methods
    template <typename Class>
    template <typename ReturnType, typename... Args>
    inline TypeRegistrar<Class>& TypeRegistrar<Class>::method(
        const std::string& name, ReturnType (Class::*method_ptr)(Args...))
    {
        info.addMethod(std::make_unique<MethodInfo>(name, getTypeName<ReturnType>(),
            createParameterTypeVector<Args...>(),
            [method_ptr, name](void* obj, const std::vector<std::any>& args) -> std::any {
                auto* typed_obj = static_cast<Class*>(obj);

                // Validate argument count at runtime
                if (args.size() != sizeof...(Args)) {
                    throw std::runtime_error("Incorrect number of arguments for method '" + name
                        + "'. Expected " + std::to_string(sizeof...(Args)) + ", got "
                        + std::to_string(args.size()));
                }

                // Use index_sequence to unpack arguments
                return callMethodImpl(
                    typed_obj, method_ptr, args, std::index_sequence_for<Args...> {});
            }));
        return *this;
    }

    // Variadic method registration for const methods
    template <typename Class>
    template <typename ReturnType, typename... Args>
    inline TypeRegistrar<Class>& TypeRegistrar<Class>::method(
        const std::string& name, ReturnType (Class::*method_ptr)(Args...) const)
    {
        info.addMethod(std::make_unique<MethodInfo>(name, getTypeName<ReturnType>(),
            createParameterTypeVector<Args...>(),
            [method_ptr, name](void* obj, const std::vector<std::any>& args) -> std::any {
                auto* typed_obj = static_cast<Class*>(obj);

                // Validate argument count at runtime
                if (args.size() != sizeof...(Args)) {
                    throw std::runtime_error("Incorrect number of arguments for method '" + name
                        + "'. Expected " + std::to_string(sizeof...(Args)) + ", got "
                        + std::to_string(args.size()));
                }

                // Use index_sequence to unpack arguments
                return callConstMethodImpl(
                    typed_obj, method_ptr, args, std::index_sequence_for<Args...> {});
            }));
        return *this;
    }

    // Helper to create parameter type vector for constructors
    template <typename... Args> std::vector<std::string> createConstructorParameterTypes()
    {
        if constexpr (sizeof...(Args) == 0) {
            return std::vector<std::string> {};
        } else {
            return std::vector<std::string> { getTypeName<Args>()... };
        }
    }

    // Helper to construct object from arguments
    template <typename Class, typename... Args, std::size_t... I>
    inline void* constructImpl(const std::vector<std::any>& args, std::index_sequence<I...>)
    {
        return new Class(std::any_cast<Args>(args[I])...);
    }

    // Constructor registration
    template <typename Class>
    template <typename... Args>
    inline TypeRegistrar<Class>& TypeRegistrar<Class>::constructor()
    {
        info.addConstructor(std::make_unique<ConstructorInfo>(
            createConstructorParameterTypes<Args...>(),
            [](const std::vector<std::any>& args) -> void* {
                // Validate argument count at runtime
                if (args.size() != sizeof...(Args)) {
                    throw std::runtime_error("Incorrect number of constructor arguments. Expected "
                        + std::to_string(sizeof...(Args)) + ", got " + std::to_string(args.size()));
                }

                // Use index_sequence to unpack arguments
                return constructImpl<Class, Args...>(args, std::index_sequence_for<Args...> {});
            }));
        return *this;
    }

} // namespace rosetta