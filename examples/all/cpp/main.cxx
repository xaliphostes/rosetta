#include "../third/Model.h"
#include "../third/Point.h"
#include "../third/Surface.h"
#include "../third/Triangle.h"
#include <iostream>

int main() {
    Surface surface({0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1}, {0, 1, 2});

    std::cout << "" << std::endl;

    for (const auto &p : surface.getPoints()) {
        std::cout << "Point(" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
    }

    std::cout << "" << std::endl;

    for (const auto &t : surface.getTriangles()) {
        std::cout << "Triangle(" << t.a << ", " << t.b << ", " << t.c << ")" << std::endl;
    }

    surface.transform([](const Point &p) { return Point(p.x, p.y, 100 * p.z); });

    Model model;
    model.addSurface(surface);

    std::cout << "" << std::endl;

    for (const auto &s : model.getSurfaces()) {
        for (const auto &p : s.getPoints()) {
            std::cout << p.x << " " << p.y << " " << p.z << std::endl;
        }
        for (const auto &t : s.getTriangles()) {
            std::cout << t.a << " " << t.b << " " << t.c << std::endl;
        }
    }

    return 0;
}