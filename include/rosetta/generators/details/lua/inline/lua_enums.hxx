/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */

namespace rosetta {

    template <typename EnumType> inline void registerEnumType(sol::state &lua) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        const auto *enum_info = EnumRegistry::instance().getEnumInfo<EnumType>();
        if (!enum_info) {
            throw std::runtime_error("Enum not registered");
        }

        // Create Lua table for enum
        sol::table enum_table = lua.create_table();

        // Add all enum values
        for (const auto &value_info : enum_info->values) {
            using UnderlyingType = std::underlying_type_t<EnumType>;
            EnumType enum_val =
                static_cast<EnumType>(static_cast<UnderlyingType>(value_info.value));
            enum_table[value_info.name] = enum_val;
        }

        // Make table read-only using metatable
        sol::table metatable                     = lua.create_table();
        metatable[sol::meta_function::new_index] = [](sol::this_state, sol::object, sol::object,
                                                      sol::object) {
            throw std::runtime_error("Cannot modify enum table");
        };
        enum_table[sol::metatable_key] = metatable;

        // Set in global scope
        lua[enum_info->name] = enum_table;

        // Register the enum type itself for type conversions
        lua.new_enum<EnumType>(
            enum_info->name + "_internal",
            [enum_info](const std::pair<sol::object, sol::object> &pair) {
                // This creates the actual enum bindings
                std::vector<std::pair<std::string, EnumType>> values;
                for (const auto &value_info : enum_info->values) {
                    using UnderlyingType = std::underlying_type_t<EnumType>;
                    values.push_back(
                        {value_info.name,
                         static_cast<EnumType>(static_cast<UnderlyingType>(value_info.value))});
                }
                return values;
            });
    }

    template <typename... EnumTypes> inline void registerEnumTypes(sol::state &lua) {
        (registerEnumType<EnumTypes>(lua), ...);
    }

    inline void bindAllEnums(sol::state &lua) {
        auto &registry = EnumRegistry::instance();

        for (const auto &enum_name : registry.getAllEnumNames()) {
            const auto *enum_info = registry.getEnumInfo(enum_name);
            if (!enum_info)
                continue;

            // Create Lua table
            sol::table enum_table = lua.create_table();

            for (const auto &value_info : enum_info->values) {
                enum_table[value_info.name] = value_info.value;
            }

            // Make read-only
            sol::table metatable                     = lua.create_table();
            metatable[sol::meta_function::new_index] = [](sol::this_state, sol::object, sol::object,
                                                          sol::object) {
                throw std::runtime_error("Cannot modify enum table");
            };
            enum_table[sol::metatable_key] = metatable;

            lua[enum_name] = enum_table;
        }
    }

} // namespace rosetta