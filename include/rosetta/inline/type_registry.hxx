/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#include <typeinfo>

namespace rosetta {

    inline TypeNameRegistry& TypeNameRegistry::instance()
    {
        static TypeNameRegistry registry;
        return registry;
    }

    inline TypeNameRegistry::TypeNameRegistry()
    {
        // Auto-register common vector types on construction
        register_common_vector_types();
    }

    inline void TypeNameRegistry::register_common_vector_types()
    {
        // Register basic vector types with their mangled names
        // This allows them to work automatically without explicit registration

        // Integer types
        register_type<std::vector<int>>(typeid(std::vector<int>).name());
        register_type<std::vector<unsigned int>>(typeid(std::vector<unsigned int>).name());
        register_type<std::vector<int32_t>>(typeid(std::vector<int32_t>).name());
        register_type<std::vector<uint32_t>>(typeid(std::vector<uint32_t>).name());
        register_type<std::vector<int64_t>>(typeid(std::vector<int64_t>).name());
        register_type<std::vector<uint64_t>>(typeid(std::vector<uint64_t>).name());
        register_type<std::vector<size_t>>(typeid(std::vector<size_t>).name());

        // Floating point types
        register_type<std::vector<float>>(typeid(std::vector<float>).name());
        register_type<std::vector<double>>(typeid(std::vector<double>).name());

        // Other basic types
        register_type<std::vector<bool>>(typeid(std::vector<bool>).name());
        register_type<std::vector<char>>(typeid(std::vector<char>).name());
        register_type<std::vector<std::string>>(typeid(std::vector<std::string>).name());
    }

    template <typename T> inline void TypeNameRegistry::register_type(const std::string& name)
    {
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
        type_names[std::type_index(typeid(BaseType))] = name;
    }

    template <typename T> inline std::string TypeNameRegistry::get_name() const
    {
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
        auto it = type_names.find(std::type_index(typeid(BaseType)));
        return (it != type_names.end()) ? it->second : "";
    }

    template <typename T> inline bool TypeNameRegistry::is_registered() const
    {
        using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
        return type_names.find(std::type_index(typeid(BaseType))) != type_names.end();
    }

    inline std::vector<std::string> TypeNameRegistry::get_all_registered_types() const
    {
        std::vector<std::string> names;
        names.reserve(type_names.size());
        for (const auto& [idx, name] : type_names) {
            names.push_back(name);
        }
        return names;
    }

    template <typename T> inline AutoTypeRegistrar<T>::AutoTypeRegistrar(const std::string& name)
    {
        TypeNameRegistry::instance().register_type<T>(name);
    }

} // namespace rosetta