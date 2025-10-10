/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "details/py/py_generator.h"
#include "details/py/py_functions.h"
#include "details/py/py_functors.h"
#include "details/py/py_pointers.h"
#include "details/py/py_vectors.h"
//#include "details/py/py_enums.h"

/**
 * @example
 * @code{.cpp}
 * #include <rosetta/generators/py.h>
 * 
 * BEGIN_PY(myModule, gen) {
 *   gen.bind_classes<Person, Vehicle>();
 * }
 * END_PY();
 */
#define BEGIN_PY(moduleName, generatorName)                            \
    PYBIND11_MODULE(moduleName, m) {                                   \
        m.doc() = "Automatic Python bindings using C++ introspection"; \
        rosetta::PyGenerator generatorName(m);

#define END_PY() }