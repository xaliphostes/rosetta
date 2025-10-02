#pragma once
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace rosetta {

    /**
     * @brief Registry for user-defined type names
     *
     * This singleton class allows registration of custom type names instead of
     * relying on typeid(T).name() which produces mangled names.
     *
     * Usage:
     * ```cpp
     * // Register a custom type
     * TypeNameRegistry::instance().register_type<Vector3D>("Vector3D");
     *
     * // Or use the convenience macro
     * REGISTER_TYPE(Vector3D);
     *
     * // Then getTypeName<Vector3D>() will return "Vector3D"
     * ```
     */
    class TypeNameRegistry {
    public:
        static TypeNameRegistry& instance()
        {
            static TypeNameRegistry registry;
            return registry;
        }

        /**
         * @brief Register a type with a custom name
         * @tparam T The type to register
         * @param name The human-readable name for the type
         */
        template <typename T> void register_type(const std::string& name)
        {
            using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
            type_names[std::type_index(typeid(BaseType))] = name;
        }

        /**
         * @brief Get the registered name for a type
         * @tparam T The type to look up
         * @return The registered name, or empty string if not found
         */
        template <typename T> std::string get_name() const
        {
            using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
            auto it = type_names.find(std::type_index(typeid(BaseType)));
            return (it != type_names.end()) ? it->second : "";
        }

        /**
         * @brief Check if a type is registered
         */
        template <typename T> bool is_registered() const
        {
            using BaseType = std::remove_cv_t<std::remove_reference_t<T>>;
            return type_names.find(std::type_index(typeid(BaseType))) != type_names.end();
        }

        /**
         * @brief Get all registered type names
         */
        std::vector<std::string> get_all_registered_types() const
        {
            std::vector<std::string> names;
            for (const auto& [idx, name] : type_names) {
                names.push_back(name);
            }
            return names;
        }

    private:
        TypeNameRegistry() = default;
        std::unordered_map<std::type_index, std::string> type_names;
    };

    // ----------------------------------------------------------------

    /**
     * @brief Auto-registration helper for introspectable classes
     *
     * This can be used in the INTROSPECTABLE macro to automatically
     * register the class type.
     */
    template <typename T> struct AutoTypeRegistrar {
        AutoTypeRegistrar(const std::string& name)
        {
            TypeNameRegistry::instance().register_type<T>(name);
        }
    };

} // namespace rosetta

/**
 * @brief Convenience macro for type registration
 *
 * Usage: REGISTER_TYPE(MyCustomClass);
 * This should be called at global scope or in an initialization function.
 */
#define REGISTER_TYPE(TypeName)                                                                    \
    namespace {                                                                                    \
        struct TypeName##_Registrar {                                                              \
            TypeName##_Registrar()                                                                 \
            {                                                                                      \
                rosetta::TypeNameRegistry::instance().register_type<TypeName>(#TypeName);          \
            }                                                                                      \
        };                                                                                         \
        static TypeName##_Registrar TypeName##_registrar_instance;                                 \
    }

/**
 * @brief Convenience macro for type registration with custom name
 *
 * Usage: REGISTER_TYPE_AS(std::vector<int>, "vector<int>");
 */
#define REGISTER_TYPE_AS(TypeName, CustomName)                                                     \
    namespace {                                                                                    \
        struct TypeName##_Registrar {                                                              \
            TypeName##_Registrar()                                                                 \
            {                                                                                      \
                rosetta::TypeNameRegistry::instance().register_type<TypeName>(CustomName);         \
            }                                                                                      \
        };                                                                                         \
        static TypeName##_Registrar TypeName##_registrar_instance;                                 \
    }

/**
 * @brief Updated INTROSPECTABLE macro with automatic type registration
 */
#define INTROSPECTABLE_WITH_AUTO_REGISTER(ClassName)                                               \
public:                                                                                            \
    static rosetta::TypeInfo& getStaticTypeInfo()                                                  \
    {                                                                                              \
        static rosetta::TypeInfo info(#ClassName);                                                 \
        static bool initialized = false;                                                           \
        if (!initialized) {                                                                        \
            rosetta::TypeNameRegistry::instance().register_type<ClassName>(#ClassName);            \
            registerIntrospection(rosetta::TypeRegistrar<ClassName>(info));                        \
            initialized = true;                                                                    \
        }                                                                                          \
        return info;                                                                               \
    }                                                                                              \
    const rosetta::TypeInfo& getTypeInfo() const override { return getStaticTypeInfo(); }          \
                                                                                                   \
private:                                                                                           \
    static void registerIntrospection(rosetta::TypeRegistrar<ClassName> reg);                      \
                                                                                                   \
public:
