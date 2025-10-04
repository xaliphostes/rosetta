/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#pragma once
#include <any>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rosetta {

    using Arg  = std::any;
    using Args = std::vector<Arg>;

    /**
     * @brief Holds information about a constructor.
     */
    class ConstructorInfo {
    public:
        std::vector<std::string>            parameter_types;
        std::function<void *(const Args &)> factory; // Creates new instance

        ConstructorInfo(const std::vector<std::string>     &param_types,
                        std::function<void *(const Args &)> fact)
            : parameter_types(param_types), factory(fact) {}
    };

    /**
     * @brief Holds information about a member variable.
     */
    class MemberInfo {
    public:
        std::string                              name;
        std::string                              type_name;
        std::function<Arg(const void *)>         getter;
        std::function<void(void *, const Arg &)> setter;

        MemberInfo(const std::string &n, const std::string &t, std::function<Arg(const void *)> g,
                   std::function<void(void *, const Arg &)> s);
    };

    /**
     * @brief Holds information about a method.
     */
    class MethodInfo {
    public:
        std::string                              name;
        std::string                              return_type;
        std::vector<std::string>                 parameter_types;
        std::function<Arg(void *, const Args &)> invoker;

        MethodInfo(const std::string &n, const std::string &ret_type,
                   const std::vector<std::string>          &param_types,
                   std::function<Arg(void *, const Args &)> inv);
    };

    /**
     * @brief Holds information about a class type, including its members and
     * methods. Uses unique_ptr to manage MemberInfo and MethodInfo instances. Copy
     * operations are deleted to prevent copying of unique_ptrs. Move operations are
     * defaulted to allow moving of TypeInfo instances. This design ensures proper
     * resource management and avoids memory leaks. C++20 standard is used for
     * std::any and other features.
     */
    class TypeInfo {
    public:
        std::string                                                  class_name;
        std::unordered_map<std::string, std::unique_ptr<MemberInfo>> members;
        std::unordered_map<std::string, std::unique_ptr<MethodInfo>> methods;
        std::vector<std::unique_ptr<ConstructorInfo>>                constructors;

        explicit TypeInfo(const std::string &name) : class_name(name) {}

        // Delete copy operations (unique_ptr is not copyable)
        TypeInfo(const TypeInfo &)            = delete;
        TypeInfo &operator=(const TypeInfo &) = delete;

        // Default move operations
        TypeInfo(TypeInfo &&)            = default;
        TypeInfo &operator=(TypeInfo &&) = default;

        void addMember(std::unique_ptr<MemberInfo> member);
        void addMethod(std::unique_ptr<MethodInfo> method);
        void addConstructor(std::unique_ptr<ConstructorInfo> ctor);

        const MemberInfo *getMember(const std::string &name) const;
        const MethodInfo *getMethod(const std::string &name) const;
        const std::vector<std::unique_ptr<ConstructorInfo>> &getConstructors() const;

        std::vector<std::string> getMemberNames() const;
        std::vector<std::string> getMethodNames() const;
    };

} // namespace rosetta

#include "inline/info.hxx"