/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <rosetta/enum_registry.h>
#include <sol/sol.hpp>

namespace rosetta {

    /**
     * @brief Register a single enum type for Lua
     */
    template <typename EnumType> void registerEnumType(sol::state &lua);

    /**
     * @brief Register multiple enum types at once
     */
    template <typename... EnumTypes> void registerEnumTypes(sol::state &lua);

    /**
     * @brief Bind all registered enums to Lua
     */
    void bindAllEnums(sol::state &lua);

} // namespace rosetta

#include "inline/lua_enums.hxx"