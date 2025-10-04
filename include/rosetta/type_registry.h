/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#pragma once
#include <functional>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rosetta {

    /**
     * @brief Registry for user-defined type names
     *
     * This singleton class allows registration of custom type names instead of
     * relying on typeid(T).name() which produces mangled names.
     */
    class TypeNameRegistry {
    public:
        static TypeNameRegistry &instance();

        /**
         * @brief Register a type with a custom name
         * @tparam T The type to register
         * @param name The human-readable name for the type
         */
        template <typename T> void register_type(const std::string &name);

        /**
         * @brief Get the registered name for a type
         * @tparam T The type to look up
         * @return The registered name, or empty string if not found
         */
        template <typename T> std::string get_name() const;

        /**
         * @brief Check if a type is registered
         */
        template <typename T> bool is_registered() const;

        /**
         * @brief Get all registered type names
         */
        std::vector<std::string> get_all_registered_types() const;

        /**
         * @brief Auto-register common vector types with their mangled names
         * This is called automatically during static initialization
         */
        void register_common_vector_types();

    private:
        TypeNameRegistry();
        std::unordered_map<std::type_index, std::string> type_names;
    };

    // ----------------------------------------------------------------

    /**
     * @brief Auto-registration helper for introspectable classes
     */
    template <typename T> struct AutoTypeRegistrar {
        AutoTypeRegistrar(const std::string &name);
    };

} // namespace rosetta

/**
 * @brief Convenience macro for type registration
 *
 * Usage: REGISTER_TYPE(MyCustomClass);
 */
#define REGISTER_TYPE(TypeName)                                                           \
    namespace {                                                                           \
        struct TypeName##_Registrar {                                                     \
            TypeName##_Registrar() {                                                      \
                rosetta::TypeNameRegistry::instance().register_type<TypeName>(#TypeName); \
            }                                                                             \
        };                                                                                \
        static TypeName##_Registrar TypeName##_registrar_instance;                        \
    }

/**
 * @brief Register a type with its mangled name from typeid
 *
 * Usage: REGISTER_TYPE_MANGLED(std::vector<double>);
 * This registers the type using typeid(T).name() as its key
 */
#define REGISTER_TYPE_MANGLED(TypeName)                                           \
    namespace {                                                                   \
        struct TypeName##_MangledRegistrar {                                      \
            TypeName##_MangledRegistrar() {                                       \
                rosetta::TypeNameRegistry::instance().register_type<TypeName>(    \
                    typeid(TypeName).name());                                     \
            }                                                                     \
        };                                                                        \
        static TypeName##_MangledRegistrar TypeName##_mangled_registrar_instance; \
    }

/**
 * @brief Register a type alias to use mangled name of the alias itself
 *
 * For type aliases like:
 *   using Vertices = std::vector<double>;
 *
 * Usage: REGISTER_TYPE_ALIAS_MANGLED(Vertices);
 * This registers Vertices using typeid(Vertices).name()
 */
#define REGISTER_TYPE_ALIAS_MANGLED(AliasName)                                                 \
    namespace {                                                                                \
        struct AliasName##_AliasMangledRegistrar {                                             \
            AliasName##_AliasMangledRegistrar() {                                              \
                rosetta::TypeNameRegistry::instance().register_type<AliasName>(                \
                    typeid(AliasName).name());                                                 \
            }                                                                                  \
        };                                                                                     \
        static AliasName##_AliasMangledRegistrar AliasName##_alias_mangled_registrar_instance; \
    }

/**
 * @brief Register vector type with automatic mangled name
 *
 * Usage: REGISTER_VECTOR_TYPE(double);
 * This registers std::vector<double> using typeid(std::vector<double>).name()
 */
#define REGISTER_VECTOR_TYPE(ElementType)                                                      \
    namespace {                                                                                \
        struct Vector_##ElementType##_Registrar {                                              \
            Vector_##ElementType##_Registrar() {                                               \
                rosetta::TypeNameRegistry::instance().register_type<std::vector<ElementType>>( \
                    typeid(std::vector<ElementType>).name());                                  \
            }                                                                                  \
        };                                                                                     \
        static Vector_##ElementType##_Registrar vector_##ElementType##_registrar_instance;     \
    }

/**
 * @brief Updated INTROSPECTABLE macro with automatic type registration
 */
#define INTROSPECTABLE_WITH_AUTO_REGISTER(ClassName)                                    \
public:                                                                                 \
    static rosetta::TypeInfo &getStaticTypeInfo() {                                     \
        static rosetta::TypeInfo info(#ClassName);                                      \
        static bool              initialized = false;                                   \
        if (!initialized) {                                                             \
            rosetta::TypeNameRegistry::instance().register_type<ClassName>(#ClassName); \
            registerIntrospection(rosetta::TypeRegistrar<ClassName>(info));             \
            initialized = true;                                                         \
        }                                                                               \
        return info;                                                                    \
    }                                                                                   \
    const rosetta::TypeInfo &getTypeInfo() const override {                             \
        return getStaticTypeInfo();                                                     \
    }                                                                                   \
                                                                                        \
private:                                                                                \
    static void registerIntrospection(rosetta::TypeRegistrar<ClassName> reg);           \
                                                                                        \
public:

#include "inline/type_registry.hxx"