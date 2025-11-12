#pragma once

#include "Scene.h"
#include "Sphere.h"
#include "Vector3D.h"
#include <cmath>
#include <fstream>
#include <memory>
#include <string>

namespace mylib {

    /**
     * @brief Create a new Vector3D
     * @param x X coordinate (default: 0.0)
     * @param y Y coordinate (default: 0.0)
     * @param z Z coordinate (default: 0.0)
     * @return New Vector3D instance
     */
    inline Vector3D create_vector(double x = 0.0, double y = 0.0, double z = 0.0) {
        return Vector3D(x, y, z);
    }

    /**
     * @brief Calculate distance between two points
     * @param a First point
     * @param b Second point
     * @return Euclidean distance between the points
     */
    inline double distance(const Vector3D &a, const Vector3D &b) {
        Vector3D diff = b - a;
        return diff.length();
    }

    /**
     * @brief Linear interpolation between two vectors
     * @param a Start vector
     * @param b End vector
     * @param t Interpolation parameter (0.0 to 1.0)
     * @return Interpolated vector
     */
    inline Vector3D lerp(const Vector3D &a, const Vector3D &b, double t) {
        return Vector3D(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
    }

    /**
     * @brief Create a new sphere
     * @param center Center point of the sphere
     * @param radius Radius of the sphere
     * @return Shared pointer to the new Sphere
     */
    inline std::shared_ptr<Sphere> create_sphere(const Vector3D &center, double radius) {
        return std::make_shared<Sphere>(center, radius);
    }

    /**
     * @brief Load a scene from file
     * @param filename Path to the scene file
     * @return Scene object loaded from file
     * @note This is a placeholder implementation. In a real application,
     *       this would parse the file format and populate the scene.
     */
    inline Scene load_scene(const std::string &filename) {
        Scene scene;

        // Placeholder implementation
        // In a real implementation, this would:
        // 1. Open and read the file
        // 2. Parse the file format (e.g., JSON, XML, custom format)
        // 3. Create shapes based on the file contents
        // 4. Add shapes to the scene

        // For now, just return an empty scene
        // You can extend this to actually load from a file format

        std::ifstream file(filename);
        if (file.is_open()) {
            // TODO: Implement actual file parsing
            // Example: parse JSON/XML and create shapes
            file.close();
        }

        return scene;
    }

} // namespace mylib