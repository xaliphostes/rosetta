#pragma once

/**
 * @file mylib.h
 * @brief Complete geometry library
 *
 * This header includes all components of the mylib geometry library:
 * - Vector3D: 3D vector operations
 * - Matrix4x4: 4x4 transformation matrices
 * - Transform: 3D transformations (position, rotation, scale)
 * - Shape: Abstract base class for shapes
 * - Sphere: Spherical shape implementation
 * - Scene: Container for geometric objects
 * - Free functions: Utility functions for geometry operations
 */

// Core types
#include "Matrix4x4.h"
#include "Transform.h"
#include "Vector3D.h"

// Shape hierarchy
#include "Shape.h"
#include "Sphere.h"

// Scene management
#include "Scene.h"

// Free functions
#include "geometry_functions.h"

namespace mylib {

    /**
     * @brief Library version information
     */
    constexpr const char *VERSION = "1.0.0";

} // namespace mylib