/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <rosetta/enum_registry.h>

namespace rosetta {

    class JsGenerator;

    /**
     * @brief Register a single enum type for JavaScript
     * @tparam EnumType The enum type
     * @param generator The JavaScript generator
     */
    template <typename EnumType> void registerEnumType(JsGenerator &generator);

    /**
     * @brief Register multiple enum types at once
     * @tparam EnumTypes The enum types
     * @param generator The JavaScript generator
     */
    template <typename... EnumTypes> void registerEnumTypes(JsGenerator &generator);

    /**
     * @brief Bind all registered enums to JavaScript
     * Creates JavaScript objects with enum values as properties
     */
    void bindAllEnums(JsGenerator &generator);

} // namespace rosetta

#include "inline/js_enums.hxx"