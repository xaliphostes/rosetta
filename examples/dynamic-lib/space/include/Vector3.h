#pragma once

// Pure, stock C++ — NO rosetta include, NO annotations. This header only
// *declares* the API; every method body lives in src/Vector3.cpp and is
// compiled into the shared library (bin/libspace.*). Rosetta reflects over
// these declarations; the generated bindings link against the shared library
// to reach the definitions.

namespace space {

    /// A point / direction in 3-D Euclidean space.
    class Vector3 {
    public:
        double x;
        double y;
        double z;

        Vector3();
        Vector3(double x, double y, double z);

        /// Euclidean length (magnitude) of the vector.
        double length() const;

        /// Dot (scalar) product with another vector.
        double dot(const Vector3 &other) const;

        /// Cross (vector) product with another vector.
        Vector3 cross(const Vector3 &other) const;

        /// A unit vector pointing in the same direction (length 1).
        Vector3 normalized() const;

        /// Component-wise sum.
        Vector3 add(const Vector3 &other) const;

        /// Uniform scaling by a scalar factor.
        Vector3 scale(double factor) const;
    };

} // namespace space
