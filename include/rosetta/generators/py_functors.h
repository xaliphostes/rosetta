/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 * Python Functor/Lambda Support
 * Allows C++ lambdas to be passed to Python and Python callables to be used in C++
 */
#pragma once

namespace rosetta {

    class PyGenerator;

    /**
     * @brief Register functor converters with the generator
     */
    inline void registerFunctorSupport(PyGenerator& generator);

    /**
     * @brief Register specific functor type converter (bidirectional)
     * @tparam Ret Return type
     * @tparam Args Argument types
     */
    template <typename Ret, typename... Args> void registerFunctorType(PyGenerator& generator);

} // namespace rosetta

#include "inline/py_functors.hxx"