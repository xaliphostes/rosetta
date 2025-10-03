/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <rosetta/generators/details/py/py_generator.h>
#include <rosetta/type_registry.h>
#include <type_traits>
#include <typeinfo>

namespace rosetta {

    // ============================================================================
    // Python-specific type conversion helpers
    // ============================================================================

    /**
     * @brief Register vector type converter for Python using automatic type name
     * @tparam T Element type of the vector
     * @param generator The Python generator to register with
     */
    template <typename T> inline void registerVectorType(PyGenerator& generator)
    {
        // This is actually handled automatically by pybind11 for most types
        // pybind11 has built-in support for std::vector<T>
        // This function is here for API consistency with JS and Lua

        // The type will be automatically converted by pybind11's built-in converters
        // No explicit registration needed unless you want custom behavior
    }

    /**
     * @brief Register type alias for Python
     * @tparam AliasType The type alias
     * @tparam ElementType The element type of the underlying vector
     * @param generator The Python generator (not used, here for consistency)
     *
     * Note: pybind11 automatically handles type aliases that resolve to
     * std::vector<T> for basic types. This function exists for API consistency.
     */
    template <typename AliasType, typename ElementType>
    inline void registerTypeAlias(PyGenerator& generator)
    {
        // pybind11 automatically handles std::vector conversions
        // Type aliases like "using Vertices = std::vector<double>"
        // work automatically without explicit registration
    }

    /**
     * @brief Register all common vector types (no-op for Python)
     * @param generator The Python generator
     *
     * pybind11 has built-in support for std::vector of most basic types,
     * so explicit registration is not needed.
     */
    inline void registerCommonVectorTypes(PyGenerator& generator)
    {
        // No-op: pybind11 handles this automatically
        // This function exists for API consistency with JavaScript and Lua
    }

    /**
     * @brief Register custom vector type with explicit converters
     * @tparam T Element type
     * @param m Python module
     * @param type_name Name for the Python type
     *
     * Use this for custom types that pybind11 doesn't handle automatically
     */
    template <typename T>
    inline void registerCustomVectorType(py::module_& m, const std::string& type_name)
    {
        py::class_<std::vector<T>>(m, type_name.c_str())
            .def(py::init<>())
            .def(py::init<size_t>())
            .def("__len__", [](const std::vector<T>& v) { return v.size(); })
            .def("__getitem__",
                [](const std::vector<T>& v, size_t i) {
                    if (i >= v.size())
                        throw py::index_error();
                    return v[i];
                })
            .def("__setitem__",
                [](std::vector<T>& v, size_t i, const T& value) {
                    if (i >= v.size())
                        throw py::index_error();
                    v[i] = value;
                })
            .def("append", [](std::vector<T>& v, const T& value) { v.push_back(value); })
            .def("clear", &std::vector<T>::clear)
            .def(
                "__iter__",
                [](const std::vector<T>& v) { return py::make_iterator(v.begin(), v.end()); },
                py::keep_alive<0, 1>());
    }

} // namespace rosetta

// ============================================================================
// USAGE NOTES FOR PYTHON
// ============================================================================
/*

Python bindings are simpler than JavaScript because pybind11 automatically
handles std::vector conversions for most basic types.

BASIC USAGE (No explicit registration needed):
----------------------------------------------

    #include <rosetta/generators/py.h>

    using Vertices = std::vector<double>;
    using Triangles = std::vector<size_t>;

    REGISTER_TYPE_ALIAS_MANGLED(Vertices);
    REGISTER_TYPE_ALIAS_MANGLED(Triangles);

    PYBIND11_MODULE(surface, m) {
        rosetta::PyGenerator generator(m);
        generator.bind_class<Surface>();
    }

    // Python automatically handles:
    # s = surface.Surface([1.0, 2.0, 3.0], [0, 1, 2])
    # print(s.vertices())  # [1.0, 2.0, 3.0]


CUSTOM TYPES (Need explicit registration):
-------------------------------------------

If you have a custom type like Point3D:

    struct Point3D {
        double x, y, z;
    };

    PYBIND11_MODULE(geometry, m) {
        // Register Point3D
        py::class_<Point3D>(m, "Point3D")
            .def(py::init<double, double, double>())
            .def_readwrite("x", &Point3D::x)
            .def_readwrite("y", &Point3D::y)
            .def_readwrite("z", &Point3D::z);

        // pybind11 automatically handles std::vector<Point3D>
        // after Point3D is registered!

        rosetta::PyGenerator generator(m);
        generator.bind_class<Mesh>();
    }

    # Python usage:
    # points = [geometry.Point3D(0,0,0), geometry.Point3D(1,1,1)]
    # mesh = geometry.Mesh(points)


ADVANCED: Custom vector behavior
---------------------------------

If you need custom vector behavior (rare):

    PYBIND11_MODULE(custom, m) {
        rosetta::registerCustomVectorType<MyType>(m, "MyTypeVector");

        rosetta::PyGenerator generator(m);
        generator.bind_class<MyClass>();
    }


COMPARISON: JavaScript vs Python
---------------------------------

JavaScript:
    // Need explicit converters
    rosetta::registerTypeAlias<Vertices, double>(generator);

Python:
    // Works automatically via pybind11
    rosetta::PyGenerator generator(m);
    generator.bind_class<Surface>();

*/