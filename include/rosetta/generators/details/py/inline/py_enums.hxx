/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#include "../py_generator.h"

namespace rosetta {

    template <typename EnumType> inline void registerEnumType(PyGenerator &generator) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        const auto *enum_info = EnumRegistry::instance().getEnumInfo<EnumType>();
        if (!enum_info) {
            throw std::runtime_error("Enum not registered");
        }

        // pybind11 has built-in enum support
        py::enum_<EnumType> py_enum(generator.module, enum_info->name.c_str());

        // Add all values
        for (const auto &value_info : enum_info->values) {
            using UnderlyingType = std::underlying_type_t<EnumType>;
            EnumType enum_val =
                static_cast<EnumType>(static_cast<UnderlyingType>(value_info.value));
            py_enum.value(value_info.name.c_str(), enum_val);
        }

        // Export values to module scope (optional, but convenient)
        py_enum.export_values();
    }

    template <typename... EnumTypes> inline void registerEnumTypes(PyGenerator &generator) {
        (registerEnumType<EnumTypes>(generator), ...);
    }

    inline void bindAllEnums(py::module_ &m) {
        // Note: This is tricky because we need the actual enum types
        // In practice, use registerEnumType<T> explicitly or registerEnumTypes<T1, T2, ...>

        // This function is mainly for consistency with JavaScript API
        // Users should explicitly call registerEnumType for each enum
    }

} // namespace rosetta