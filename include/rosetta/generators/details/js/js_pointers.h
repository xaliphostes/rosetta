/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once

namespace rosetta {

    class JsGenerator;

    /**
     * @brief Register pointer type converter for any introspectable class
     * @tparam T The introspectable class type
     * @param generator The JavaScript generator to register with
     */
    template <typename T> void registerPointerType(JsGenerator& generator);

    /**
     * @brief Register pointer converters for multiple classes
     * @tparam Classes The introspectable class types
     * @param generator The JavaScript generator to register with
     */
    template <typename... Classes> void registerPointerTypes(JsGenerator& generator);

} // namespace rosetta

#include "inline/js_pointers.hxx"
