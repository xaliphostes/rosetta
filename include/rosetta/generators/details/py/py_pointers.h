/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once

namespace rosetta {

    template <typename T> inline void registerPointerType(PyGenerator& generator)
    {
        // pybind11 handles pointers automatically in most cases
        // This is mainly for explicit control if needed
    }

} // namespace rosetta
