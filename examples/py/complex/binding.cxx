#include <cmath>
#include <rosetta/extensions/generators/py_generator.h>
// #include <rosetta/extensions/generators/py_vector_type.h>
#include <rosetta/rosetta.h>

struct Point {
    double x, y, z;
    Point() : x(0), y(0), z(0) {}
    Point(double x, double y, double z) : x(x), y(y), z(z) {}
};

// -----------------------------------------------------

struct Triangle {
    int a, b, c;
    Triangle() : a(0), b(0), c(0) {}
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};

// -----------------------------------------------------

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

// -----------------------------------------------------

class Model {
    std::vector<Surface> surfaces;

public:
    Model() = default;
    void                        addSurface(const Surface &surface) { surfaces.push_back(surface); }
    const std::vector<Surface> &getSurfaces() const { return surfaces; }
    void                        setSurfaces(const std::vector<Surface> &s) { surfaces = s; }
};

// ============================================================================
// Registration with Rosetta (done once, typically in a registration file)
// ============================================================================

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
    // (it create a virtual field).

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

// ============================================================================
// Python Module Definition
// ============================================================================

BEGIN_PY_MODULE(rosetta_example, "Python bindings for C++ classes using Rosetta introspection") {
    register_rosetta_classes();

    // *** IMPORTANT: Register function converters BEFORE binding classes ***
    // Register the std::function<Point(const Point&)> type converter
    // This allows Python lambdas to be converted to C++ std::function
    rosetta::py::register_function_converter<Point, const Point &>();

    BIND_PY_CLASS(Point);
    BIND_PY_CLASS(Triangle);
    BIND_PY_CLASS(Surface);
    BIND_PY_CLASS(Model);
}
END_PY_MODULE()