#pragma once

#include "Vector3D.h"
#include <string>
#include <vector>

namespace mylib {

    /**
     * @brief Abstract base class for geometric shapes
     */
    class Shape {
    public:
        virtual ~Shape() = default;

        /**
         * @brief Calculate surface area
         * @return Surface area of the shape
         */
        virtual double area() const = 0;

        /**
         * @brief Calculate volume
         * @return Volume of the shape
         */
        virtual double volume() const = 0;

        /**
         * @brief Get shape name
         * @return Name of the shape type
         */
        virtual std::string getName() const { return "Shape"; }

        /**
         * @brief Get bounding box corners
         * @return Vector of corner points defining the bounding box
         */
        virtual std::vector<Vector3D> boundingBox() const { return {}; }
    };

} // namespace mylib