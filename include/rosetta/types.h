/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#pragma once
#include <rosetta/info.h>
#include <rosetta/type_registry.h>

namespace rosetta {

    /**
     * @brief Holds information about a class type, including its members and
     * methods. Uses unique_ptr to manage MemberInfo and MethodInfo instances. Copy
     * operations are deleted to prevent copying of unique_ptrs. Move operations are
     * defaulted to allow moving of TypeInfo instances. This design ensures proper
     * resource management and avoids memory leaks. C++20 standard is used for
     * std::any and other features.
     */
    template <typename T> std::string getTypeName();

    /**
     * @brief Helper class to register members and methods of a class.
     * This class is used in conjunction with the INTROSPECTABLE macro to
     * register class members and methods for introspection. It provides methods
     * to add members and methods to the TypeInfo instance. The class is templated
     * on the class type to ensure type safety during registration. C++20 standard
     * is used for std::any and other features.
     */
    template <typename Class> class TypeRegistrar {
    private:
        TypeInfo &info;

    public:
        explicit TypeRegistrar(TypeInfo &type_info) : info(type_info) {}

        /**
         * @brief Register a member variable.
         * This method registers a member variable of the class by its name and
         * pointer. It creates a MemberInfo instance with appropriate getter and
         * setter functions. The getter retrieves the member's value from an
         * instance of the class, and the setter updates the member's value. The
         * member's type is deduced using the getTypeName function.
         */
        template <typename MemberType>
        TypeRegistrar &member(const std::string &name, MemberType Class::*member_ptr);

        /**
         * @brief Register a method based on C++ variadic method registration
         * (handles any number of parameters). This method registers a method of the
         * class by its name and pointer. It creates a MethodInfo instance with an
         * invoker function that calls the method on an instance of the class. The
         * method's return type and parameter types are deduced using the
         * getTypeName function.
         */
        template <typename ReturnType, typename... Args>
        TypeRegistrar &method(const std::string &name, ReturnType (Class::*method_ptr)(Args...));

        /**
         * @brief Register a const method based on C++ variadic method registration
         * (handles any number of parameters).
         */
        template <typename ReturnType, typename... Args>
        TypeRegistrar &method(const std::string &name,
                              ReturnType (Class::*method_ptr)(Args...) const);

        /**
         * @brief Register a constructor with specific parameter types.
         * This creates a factory function that constructs the object.
         */
        template <typename... Args> TypeRegistrar &constructor();
    };

} // namespace rosetta

#include "inline/types.hxx"