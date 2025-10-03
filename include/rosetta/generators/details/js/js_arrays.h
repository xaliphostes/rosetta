/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once

namespace rosetta {

    class JsGenerator;

    // ============================================================================
    // Auto array registration - Public API
    // ============================================================================

    /**
     * @brief Register std::array type converter using automatic type name from typeid
     * @tparam T Element type of the array
     * @tparam N Size of the array
     * @param generator The JavaScript generator to register with
     */
    template <typename T, size_t N> void registerArrayType(JsGenerator& generator);

    /**
     * @brief Register type alias converter for std::array
     * @tparam AliasType The type alias (e.g., Vec3 = std::array<double, 3>)
     * @tparam ElementType The element type of the underlying array
     * @tparam N Size of the array
     * @param generator The JavaScript generator to register with
     */
    template <typename AliasType, typename ElementType, size_t N>
    void registerArrayAlias(JsGenerator& generator);

    /**
     * @brief Register common fixed-size array types
     * @param generator The JavaScript generator to register with
     */
    void registerCommonArrayTypes(JsGenerator& generator);

} // namespace rosetta

#include "inline/js_arrays.hxx"