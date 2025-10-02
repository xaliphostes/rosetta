/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
namespace rosetta {

    inline MemberInfo::MemberInfo(const std::string& n, const std::string& t,
        std::function<std::any(const void*)> g, std::function<void(void*, const Arg&)> s)
        : name(n)
        , type_name(t)
        , getter(g)
        , setter(s)
    {
    }

    inline MethodInfo::MethodInfo(const std::string& n, const std::string& ret_type,
        const std::vector<std::string>& param_types,
        std::function<std::any(void*, const Args&)> inv)
        : name(n)
        , return_type(ret_type)
        , parameter_types(param_types)
        , invoker(inv)
    {
    }

    inline void TypeInfo::addMember(std::unique_ptr<MemberInfo> member)
    {
        members[member->name] = std::move(member);
    }

    inline void TypeInfo::addMethod(std::unique_ptr<MethodInfo> method)
    {
        methods[method->name] = std::move(method);
    }

    inline void TypeInfo::addConstructor(std::unique_ptr<ConstructorInfo> ctor)
    {
        constructors.push_back(std::move(ctor));
    }

    inline const MemberInfo* TypeInfo::getMember(const std::string& name) const
    {
        auto it = members.find(name);
        return (it != members.end()) ? it->second.get() : nullptr;
    }

    inline const MethodInfo* TypeInfo::getMethod(const std::string& name) const
    {
        auto it = methods.find(name);
        return (it != methods.end()) ? it->second.get() : nullptr;
    }

    inline const std::vector<std::unique_ptr<ConstructorInfo>>& TypeInfo::getConstructors() const
    {
        return constructors;
    }

    inline std::vector<std::string> TypeInfo::getMemberNames() const
    {
        std::vector<std::string> names;
        for (const auto& pair : members) {
            names.push_back(pair.first);
        }
        return names;
    }

    inline std::vector<std::string> TypeInfo::getMethodNames() const
    {
        std::vector<std::string> names;
        for (const auto& pair : methods) {
            names.push_back(pair.first);
        }
        return names;
    }

}