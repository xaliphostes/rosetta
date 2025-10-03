/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once

namespace rosetta {

    class JsGenerator;

    // ============================================================================
    // Auto vector registration - Public API
    // ============================================================================

    /**
     * @brief Register vector type converter using automatic type name from typeid
     * @tparam T Element type of the vector
     * @param generator The JavaScript generator to register with
     */
    template <typename T> inline void registerVectorType(JsGenerator& generator);

    /**
     * @brief Register type alias converter (for aliases like "using Vertices =
     * std::vector<double>")
     * @tparam AliasType The type alias
     * @tparam ElementType The element type of the underlying vector
     * @param generator The JavaScript generator to register with
     */
    template <typename AliasType, typename ElementType>
    inline void registerTypeAlias(JsGenerator& generator);

    /**
     * @brief Register all common vector types
     * @param generator The JavaScript generator to register with
     */
    inline void registerCommonVectorTypes(JsGenerator& generator);

} // namespace rosetta

#include "inline/js_vectors.hxx"