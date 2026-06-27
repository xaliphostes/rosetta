#pragma once

#include "Vector3.h"

// Stock C++. Declarations only — bodies in src/BoundingBox.cpp, compiled into
// the shared library. Demonstrates a class whose members are *another* user
// type (Vector3), marshalled across the language boundary by the bindings.

namespace space {

    /// An axis-aligned bounding box, defined by two opposite corners.
    class BoundingBox {
    public:
        Vector3 min;
        Vector3 max;

        BoundingBox();
        BoundingBox(const Vector3 &min, const Vector3 &max);

        /// The geometric centre of the box.
        Vector3 center() const;

        /// The extent of the box along each axis (max - min).
        Vector3 size() const;

        /// The length of the main diagonal.
        double diagonal() const;

        /// True if the given point lies inside (or on) the box.
        bool contains(const Vector3 &point) const;
    };

} // namespace space
