/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <pybind11/pybind11.h>
#include <rosetta/enum_registry.h>

namespace py = pybind11;

namespace rosetta {

    class PyGenerator;

    /**
     * @brief Register a single enum type for Python
     * Creates a Python enum.IntEnum subclass
     */
    template <typename EnumType> void registerEnumType(PyGenerator &generator);

    /**
     * @brief Register multiple enum types at once
     */
    template <typename... EnumTypes> void registerEnumTypes(PyGenerator &generator);

    /**
     * @brief Bind all registered enums to Python
     */
    void bindAllEnums(py::module_ &m);

} // namespace rosetta

#include "inline/py_enums.hxx"