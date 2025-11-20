#include <rosetta/rosetta.h>
#include "third/common.h"

// Setting up the Rosetta introspection for all bindings (Python, JavaScript, Emscripten, ...)

void register_rosetta_classes() {
    ROSETTA_REGISTER_CLASS(Point)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Point::x)
        .field("y", &Point::y)
        .field("z", &Point::z);

    ROSETTA_REGISTER_CLASS(Triangle)
        .constructor<>()
        .constructor<int, int, int>()
        .field("a", &Triangle::a)
        .field("b", &Triangle::b)
        .field("c", &Triangle::c);

    // Here the "property" replaces the two methods "get..." and "set..."
    // (it create a virtual "field").

    ROSETTA_REGISTER_CLASS(Surface)
        .constructor<>()
        .constructor<const std::vector<double> &, const std::vector<int> &>()
        .property("points", &Surface::getPoints, &Surface::setPoints)
        .property("triangles", &Surface::getTriangles, &Surface::setTriangles)
        .method("transform", &Surface::transform);

    ROSETTA_REGISTER_CLASS(Model)
        .property("surfaces", &Model::getSurfaces, &Model::setSurfaces)
        .method("addSurface", &Model::addSurface);
}

