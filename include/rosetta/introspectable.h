/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#pragma once
#include <rosetta/info.h>
#include <rosetta/types.h>

namespace rosetta {

    /**
     * Base class for introspectable objects.
     * Classes inheriting from Introspectable must implement the getTypeInfo()
     * method to provide their TypeInfo instance. This design allows runtime
     * introspection of class members and methods. C++20 standard is used for
     * std::any and other features.
     */
    class Introspectable {
    public:
        virtual ~Introspectable()                   = default;
        virtual const TypeInfo &getTypeInfo() const = 0;

        // Introspection utility methods
        std::any                 getMemberValue(const std::string &member_name) const;
        void                     setMemberValue(const std::string &member_name, const Arg &value);
        std::any                 callMethod(const std::string &method_name, const Args &args = {});
        std::vector<std::string> getMemberNames() const;
        std::vector<std::string> getMethodNames() const;
        std::string              getClassName() const;
        bool                     hasMember(const std::string &name) const;
        bool                     hasMethod(const std::string &name) const;

        void printMemberValue(const std::string &member_name) const;
        void printClassInfo() const;

        std::string toJSON() const;
    };

} // namespace rosetta

/**
 * @brief Macro to declare a class as introspectable.
 * This macro should be placed in the public section of the class definition. It
 * defines the necessary static and instance methods to provide TypeInfo for
 * the class. The static method getStaticTypeInfo() initializes and returns a
 * singleton TypeInfo instance for the class. The instance method getTypeInfo()
 * overrides the pure virtual method from Introspectable to return the static
 * TypeInfo. The macro also declares a private static method
 * registerIntrospection() that must be implemented by the user to register the
 * class's members and methods. This design ensures that each introspectable
 * class has a single TypeInfo instance, avoiding redundant copies and ensuring
 * efficient memory usage. C++20 standard is used for std::any and other
 * features.
 */
#define INTROSPECTABLE(ClassName)                                             \
public:                                                                       \
    static rosetta::TypeInfo &getStaticTypeInfo() {                           \
        static rosetta::TypeInfo info(#ClassName);                            \
        static bool              initialized = false;                         \
        if (!initialized) {                                                   \
            registerIntrospection(rosetta::TypeRegistrar<ClassName>(info));   \
            initialized = true;                                               \
        }                                                                     \
        return info;                                                          \
    }                                                                         \
    const rosetta::TypeInfo &getTypeInfo() const override {                   \
        return getStaticTypeInfo();                                           \
    }                                                                         \
                                                                              \
private:                                                                      \
    static void registerIntrospection(rosetta::TypeRegistrar<ClassName> reg); \
                                                                              \
public:

#include "inline/introspectable.hxx"
