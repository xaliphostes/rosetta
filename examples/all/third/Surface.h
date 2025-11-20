#pragma once
#include <vector>
#include "Point.h"
#include "Triangle.h"

class Surface {
    std::vector<Point>    points;
    std::vector<Triangle> triangles;

public:
    Surface() = default;
    Surface(const std::vector<double> &positions, const std::vector<int> &indices) {
        size_t num_points = positions.size() / 3;
        for (size_t i = 0; i < num_points; ++i) {
            size_t idx = 3 * i;
            points.push_back(Point{positions[idx], positions[idx + 1], positions[idx + 2]});
        }

        size_t num_triangles = indices.size() / 3;
        for (size_t i = 0; i < num_triangles; ++i) {
            size_t idx = 3 * i;
            triangles.push_back(Triangle{indices[idx], indices[idx + 1], indices[idx + 2]});
        }
    }

    void transform(std::function<Point(const Point &)> func) {
        for (auto &p : points) {
            p = func(p);
        }
    }

    const std::vector<Point>    &getPoints() const { return points; }
    void                         setPoints(const std::vector<Point> &pts) { points = pts; }
    const std::vector<Triangle> &getTriangles() const { return triangles; }
    void setTriangles(const std::vector<Triangle> &tris) { triangles = tris; }
};