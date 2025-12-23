#include "../registration.h"
#include "../third/Model.h"
#include "../third/Point.h"
#include "../third/Surface.h"
#include "../third/Triangle.h"
#include <iostream>

int main() {
    register_rosetta_classes();

    // Get metadata
    auto &surface_meta  = ROSETTA_GET_META(Surface);
    auto &point_meta    = ROSETTA_GET_META(Point);
    auto &triangle_meta = ROSETTA_GET_META(Triangle);
    auto &model_meta    = ROSETTA_GET_META(Model);

    auto surface_any = surface_meta.constructors()[1]({
        std::vector<double>{0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1},
        std::vector<int>{0, 1, 2}
    });
    Surface &surface = surface_any.as<Surface>();

    std::cout << "" << std::endl;

    auto  points_any = surface_meta.get_field(surface, "points");
    auto &points     = points_any.as<std::vector<Point>>();
    for (auto &p : points) {
        auto x = point_meta.get_field(p, "x").as<double>();
        auto y = point_meta.get_field(p, "y").as<double>();
        auto z = point_meta.get_field(p, "z").as<double>();
        std::cout << "Point(" << x << ", " << y << ", " << z << ")" << std::endl;
    }

    std::cout << "" << std::endl;

    auto  triangles_any = surface_meta.get_field(surface, "triangles");
    auto &triangles     = triangles_any.as<std::vector<Triangle>>();
    for (auto &t : triangles) {
        auto a = triangle_meta.get_field(t, "a").as<int>();
        auto b = triangle_meta.get_field(t, "b").as<int>();
        auto c = triangle_meta.get_field(t, "c").as<int>();
        std::cout << "Triangle(" << a << ", " << b << ", " << c << ")" << std::endl;
    }

    std::function<Point(const Point &)> transform_func = [&](const Point &p) {
        auto x = point_meta.get_field(const_cast<Point &>(p), "x").as<double>();
        auto y = point_meta.get_field(const_cast<Point &>(p), "y").as<double>();
        auto z = point_meta.get_field(const_cast<Point &>(p), "z").as<double>();

        auto new_point_any = point_meta.constructors()[1]({x, y, 100 * z});
        return new_point_any.as<Point>();
    };

    surface_meta.invoke_method(surface, "transform", {transform_func});

    auto model_any = model_meta.constructors()[0]({});
    Model& model = model_any.as<Model>();
    
    model_meta.invoke_method(model, "addSurface", {surface});

    std::cout << "" << std::endl;

    auto  surfaces_any = model_meta.get_field(model, "surfaces");
    auto &surfaces     = surfaces_any.as<std::vector<Surface>>();
    for (auto &s : surfaces) {
        auto  s_points_any = surface_meta.get_field(s, "points");
        auto &s_points     = s_points_any.as<std::vector<Point>>();
        for (auto &p : s_points) {
            auto x = point_meta.get_field(p, "x").as<double>();
            auto y = point_meta.get_field(p, "y").as<double>();
            auto z = point_meta.get_field(p, "z").as<double>();
            std::cout << x << " " << y << " " << z << std::endl;
        }
        auto  s_triangles_any = surface_meta.get_field(s, "triangles");
        auto &s_triangles     = s_triangles_any.as<std::vector<Triangle>>();
        for (auto &t : s_triangles) {
            auto a = triangle_meta.get_field(t, "a").as<int>();
            auto b = triangle_meta.get_field(t, "b").as<int>();
            auto c = triangle_meta.get_field(t, "c").as<int>();
            std::cout << a << " " << b << " " << c << std::endl;
        }
    }

    return 0;
}