/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 * JavaScript Functor/Lambda Support
 * Allows C++ lambdas to be passed to JavaScript for use with forEach, map, reduce, etc.
 */
#pragma once

namespace rosetta {

    class JsGenerator;

    /**
     * @brief Register functor converters with the generator
     */
    inline void registerFunctorSupport(JsGenerator& generator);

    /**
     * @brief Register specific functor type converter (bidirectional)
     * @tparam Ret Return type
     * @tparam Args Argument types
     */
    template <typename Ret, typename... Args>
    void registerFunctorType(JsGenerator& generator);

} // namespace rosetta

#include "inline/js_functors.hxx"