/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rosetta {

    /**
     * @brief Information about a single enum value
     */
    struct EnumValueInfo {
        std::string name;
        int64_t     value;

        EnumValueInfo(const std::string &n, int64_t v) : name(n), value(v) {}
    };

    /**
     * @brief Information about an enum type
     */
    class EnumInfo {
    public:
        std::string                              name;
        std::vector<EnumValueInfo>               values;
        std::unordered_map<std::string, int64_t> name_to_value;
        std::unordered_map<int64_t, std::string> value_to_name;

        explicit EnumInfo(const std::string &enum_name) : name(enum_name) {}

        void addValue(const std::string &value_name, int64_t value) {
            values.emplace_back(value_name, value);
            name_to_value[value_name] = value;
            value_to_name[value]      = value_name;
        }

        bool hasValue(const std::string &value_name) const {
            return name_to_value.find(value_name) != name_to_value.end();
        }

        bool hasValue(int64_t value) const {
            return value_to_name.find(value) != value_to_name.end();
        }

        int64_t getValue(const std::string &value_name) const {
            auto it = name_to_value.find(value_name);
            if (it != name_to_value.end()) {
                return it->second;
            }
            throw std::runtime_error("Enum value '" + value_name + "' not found in enum '" + name +
                                     "'");
        }

        std::string getName(int64_t value) const {
            auto it = value_to_name.find(value);
            if (it != value_to_name.end()) {
                return it->second;
            }
            throw std::runtime_error("Enum value " + std::to_string(value) +
                                     " not found in enum '" + name + "'");
        }
    };

    /**
     * @brief Registry for enum types
     */
    class EnumRegistry {
    public:
        static EnumRegistry &instance();

        template <typename EnumType> void registerEnum(const std::string &enum_name);

        template <typename EnumType>
        void addEnumValue(const std::string &value_name, EnumType value);

        template <typename EnumType> const EnumInfo *getEnumInfo() const;

        const EnumInfo *getEnumInfo(const std::string &enum_name) const;

        template <typename EnumType> bool isRegistered() const;

        std::vector<std::string> getAllEnumNames() const;

    private:
        EnumRegistry() = default;
        std::unordered_map<std::type_index, EnumInfo>    enums_by_type;
        std::unordered_map<std::string, std::type_index> enums_by_name;
    };

    /**
     * @brief Helper class for fluent enum registration
     */
    template <typename EnumType> class EnumRegistrar {
    public:
        explicit EnumRegistrar(const std::string &enum_name) {
            EnumRegistry::instance().registerEnum<EnumType>(enum_name);
            enum_name_ = enum_name;
        }

        EnumRegistrar &value(const std::string &name, EnumType val) {
            EnumRegistry::instance().addEnumValue(name, val);
            return *this;
        }

    private:
        std::string enum_name_;
    };

} // namespace rosetta

// ============================================================================
// REGISTRATION MACROS
// ============================================================================

/**
 * @brief Primary macro for registering enum class with values
 * This is the RECOMMENDED method - most reliable
 *
 * Usage:
 * enum class Status { Active, Inactive, Pending };
 * BEGIN_ENUM_REGISTRATION(Status)
 *     ENUM_VALUE(Active)
 *     ENUM_VALUE(Inactive)
 *     ENUM_VALUE(Pending)
 * END_ENUM_REGISTRATION()
 */
#define BEGIN_ENUM_REGISTRATION(EnumType)         \
    namespace {                                   \
        struct EnumType##_Registrar {             \
            using EnumType_t = EnumType;          \
            rosetta::EnumRegistrar<EnumType> reg; \
            EnumType##_Registrar() : reg(#EnumType) {

#define ENUM_VALUE(ValueName) reg.value(#ValueName, EnumType_t::ValueName);

#define END_ENUM_REGISTRATION() \
    }                           \
    }                           \
    enum_registrar_instance;    \
    }

/**
 * @brief Alternative macro for plain enums (not enum class)
 *
 * Usage:
 * enum Color { Red, Green, Blue };
 * BEGIN_PLAIN_ENUM_REGISTRATION(Color)
 *     PLAIN_ENUM_VALUE(Red)
 *     PLAIN_ENUM_VALUE(Green)
 *     PLAIN_ENUM_VALUE(Blue)
 * END_PLAIN_ENUM_REGISTRATION()
 */
#define BEGIN_PLAIN_ENUM_REGISTRATION(EnumType)   \
    namespace {                                   \
        struct EnumType##_Registrar {             \
            rosetta::EnumRegistrar<EnumType> reg; \
            EnumType##_Registrar() : reg(#EnumType) {

#define PLAIN_ENUM_VALUE(ValueName) reg.value(#ValueName, ValueName);

#define END_PLAIN_ENUM_REGISTRATION() \
    }                                 \
    }                                 \
    enum_registrar_instance;          \
    }

/**
 * @brief Helper macro for single value registration (internal use)
 */
#define ROSETTA_ENUM_VALUE_IMPL(EnumType, ValueName) reg.value(#ValueName, EnumType::ValueName);

/**
 * @brief Convenient one-liner registration for 1-6 enum values
 *
 * Usage:
 * REGISTER_ENUM_3(Status, Active, Inactive, Pending)
 * REGISTER_ENUM_4(Priority, Low, Medium, High, Critical)
 */
#define REGISTER_ENUM_1(EnumType, a)                               \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

#define REGISTER_ENUM_2(EnumType, a, b)                            \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, b)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

#define REGISTER_ENUM_3(EnumType, a, b, c)                         \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, b)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, c)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

#define REGISTER_ENUM_4(EnumType, a, b, c, d)                      \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, b)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, c)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, d)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

#define REGISTER_ENUM_5(EnumType, a, b, c, d, e)                   \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, b)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, c)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, d)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, e)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

#define REGISTER_ENUM_6(EnumType, a, b, c, d, e, f)                \
    namespace {                                                    \
        struct EnumType##_Registrar {                              \
            EnumType##_Registrar() {                               \
                rosetta::EnumRegistrar<EnumType> reg(#EnumType);   \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, a)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, b)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, c)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, d)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, e)               \
                ROSETTA_ENUM_VALUE_IMPL(EnumType, f)               \
            }                                                      \
        };                                                         \
        static EnumType##_Registrar EnumType##_registrar_instance; \
    }

/**
 * @brief Default macro - aliases to REGISTER_ENUM_3 (most common case)
 * For other counts, use REGISTER_ENUM_1, REGISTER_ENUM_2, etc.
 */
#define REGISTER_ENUM(EnumType, a, b, c) REGISTER_ENUM_3(EnumType, a, b, c)

#include "inline/enum_registry.hxx"