/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
namespace rosetta {

    inline TypeNameRegistry& TypeNameRegistry::instance()
    {
        static TypeNameRegistry registry;
        return registry;
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