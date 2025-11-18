#pragma once

#include "Shape.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace mylib {

    /**
     * @brief Container for geometric objects
     */
    class Scene {
    private:
        std::vector<std::shared_ptr<Shape>> m_shapes;

    public:
        /**
         * @brief Create empty scene
         */
        Scene() = default;

        /**
         * @brief Add a shape to the scene
         * @param shape Shared pointer to the shape to add
         */
        void addShape(std::shared_ptr<Shape> shape) {
            if (shape) {
                m_shapes.push_back(shape);
            }
        }

        /**
         * @brief Get all shapes
         * @return Vector of all shapes in the scene
         */
        std::vector<std::shared_ptr<Shape>> getShapes() const { return m_shapes; }

        /**
         * @brief Find shape by name
         * @param name Name of the shape to find
         * @return Shared pointer to the first shape with matching name, or nullptr if not found
         */
        std::shared_ptr<Shape> findShapeByName(const std::string &name) const {
            auto it = std::find_if(m_shapes.begin(), m_shapes.end(),
                                   [&name](const std::shared_ptr<Shape> &shape) {
                                       return shape && shape->getName() == name;
                                   });

            if (it != m_shapes.end()) {
                return *it;
            }
            return nullptr;
        }

        /**
         * @brief Remove all shapes
         */
        void clear() { m_shapes.clear(); }

        /**
         * @brief Calculate total volume of all shapes
         * @return Sum of volumes of all shapes in the scene
         */
        double getTotalVolume() const {
            double total = 0.0;
            for (const auto &shape : m_shapes) {
                if (shape) {
                    total += shape->volume();
                }
            }
            return total;
        }

        /**
         * @brief Get number of objects in scene
         * @return Number of shapes in the scene
         */
        size_t getObjectCount() const { return m_shapes.size(); }
    };

} // namespace mylib