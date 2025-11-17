#pragma once

#include "Shape.h"
#include "Vector3D.h"
#include <cmath>

namespace mylib {

    /**
     * @brief Spherical shape
     */
    class Sphere : public Shape {
    private:
        Vector3D m_center;
        double   m_radius;

    public:
        /**
         * @brief Create sphere with center and radius
         * @param center Center point of the sphere
         * @param radius Radius of the sphere
         */
        Sphere(const Vector3D &center, double radius) : m_center(center), m_radius(radius) {}

        /**
         * @brief Get center of the sphere
         */
        Vector3D getCenter() const { return m_center; }

        /**
         * @brief Set center of the sphere
         */
        void setCenter(const Vector3D &center) { m_center = center; }

        /**
         * @brief Get radius of the sphere
         */
        double getRadius() const { return m_radius; }

        /**
         * @brief Set radius of the sphere
         */
        void setRadius(double radius) { m_radius = radius; }

        /**
         * @brief Calculate sphere surface area
         * @return Surface area (4 * π * r²)
         */
        double area() const override { return 4.0 * M_PI * m_radius * m_radius; }

        /**
         * @brief Calculate sphere volume
         * @return Volume (4/3 * π * r³)
         */
        double volume() const override {
            return (4.0 / 3.0) * M_PI * m_radius * m_radius * m_radius;
        }

        /**
         * @brief Returns 'Sphere'
         */
        std::string getName() const override { return "Sphere"; }

        /**
         * @brief Get bounding box corners
         * @return Vector of 8 corner points of the bounding box
         */
        std::vector<Vector3D> boundingBox() const override {
            std::vector<Vector3D> corners;
            corners.reserve(8);

            // Generate 8 corners of bounding box
            for (int i = 0; i < 8; ++i) {
                double dx = (i & 1) ? m_radius : -m_radius;
                double dy = (i & 2) ? m_radius : -m_radius;
                double dz = (i & 4) ? m_radius : -m_radius;
                corners.push_back(Vector3D(m_center.x + dx, m_center.y + dy, m_center.z + dz));
            }

            return corners;
        }

        /**
         * @brief Check if point is inside sphere
         * @param point Point to test
         * @return true if point is inside or on the sphere surface
         */
        bool containsPoint(const Vector3D &point) const {
            Vector3D diff = point - m_center;
            return diff.length() <= m_radius;
        }
    };

} // namespace mylib