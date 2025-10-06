/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
namespace rosetta {

    inline EnumRegistry &EnumRegistry::instance() {
        static EnumRegistry registry;
        return registry;
    }

    template <typename EnumType>
    inline void EnumRegistry::registerEnum(const std::string &enum_name) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        std::type_index type_idx(typeid(EnumType));

        // Create new EnumInfo if not already registered
        if (enums_by_type.find(type_idx) == enums_by_type.end()) {
            enums_by_type.emplace(type_idx, EnumInfo(enum_name));
            enums_by_name[enum_name] = type_idx;
        }
    }

    template <typename EnumType>
    inline void EnumRegistry::addEnumValue(const std::string &value_name, EnumType value) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        std::type_index type_idx(typeid(EnumType));
        auto            it = enums_by_type.find(type_idx);

        if (it != enums_by_type.end()) {
            // Cast enum to underlying type
            using UnderlyingType = std::underlying_type_t<EnumType>;
            int64_t int_value    = static_cast<int64_t>(static_cast<UnderlyingType>(value));
            it->second.addValue(value_name, int_value);
        }
    }

    template <typename EnumType> inline const EnumInfo *EnumRegistry::getEnumInfo() const {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        std::type_index type_idx(typeid(EnumType));
        auto            it = enums_by_type.find(type_idx);

        return (it != enums_by_type.end()) ? &it->second : nullptr;
    }

    inline const EnumInfo *EnumRegistry::getEnumInfo(const std::string &enum_name) const {
        auto it = enums_by_name.find(enum_name);
        if (it != enums_by_name.end()) {
            return getEnumInfo<int>(); // Placeholder, need type erasure
        }
        return nullptr;
    }

    template <typename EnumType> inline bool EnumRegistry::isRegistered() const {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        std::type_index type_idx(typeid(EnumType));
        return enums_by_type.find(type_idx) != enums_by_type.end();
    }

    inline std::vector<std::string> EnumRegistry::getAllEnumNames() const {
        std::vector<std::string> names;
        names.reserve(enums_by_name.size());

        for (const auto &[name, _] : enums_by_name) {
            names.push_back(name);
        }

        return names;
    }

} // namespace rosetta