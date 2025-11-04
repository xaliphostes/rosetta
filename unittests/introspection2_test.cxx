#include "TEST.h"

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
public:
    std::vector<Point>    points;
    std::vector<Triangle> triangles;

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

    // --- converters (get/set) exposed to Rosetta ---

    const std::vector<Point>    &getPoints() const { return points; }
    void                         setPoints(const std::vector<Point> &pts) { points = pts; }
    const std::vector<Triangle> &getTriangles() const { return triangles; }
    void setTriangles(const std::vector<Triangle> &tris) { triangles = tris; }

    void forEachPoint(const std::function<void(const Point &)> &fct) const {
        for (const auto &p : points) {
            fct(p);
        }
    }
    void forEachTriangle(const std::function<void(const Triangle &)> &fct) const {
        for (const auto &t : triangles) {
            fct(t);
        }
    }
};

// -----------------------------------------------------
// Use a wrapper IModel class to expose Model
// -----------------------------------------------------
class Model {
public:
    std::vector<Surface> surfaces;

    Model() = default;
    void                        addSurface(const Surface &surface) { surfaces.push_back(surface); }
    const std::vector<Surface> &getSurfaces() const { return surfaces; }
    void                        setSurfaces(const std::vector<Surface> &s) { surfaces = s; }
};

void initFct() {
    using namespace rosetta;

    ROSETTA_REGISTER_CLASS(Point)
        .constructor<double, double, double>()
        .field("x", &Point::x)
        .field("y", &Point::y)
        .field("z", &Point::z);

    ROSETTA_REGISTER_CLASS(Triangle)
        .constructor<int, int, int>()
        .field("a", &Triangle::a)
        .field("b", &Triangle::b)
        .field("c", &Triangle::c);

    ROSETTA_REGISTER_CLASS(Surface)
        .constructor<const std::vector<double> &, const std::vector<int> &>()
        .field("points", &Surface::points)
        .field("triangles", &Surface::triangles)
        .method("setPoints", &Surface::setPoints)
        .method("setTriangles", &Surface::setTriangles)
        .method("getPoints", &Surface::getPoints)
        .method("getTriangles", &Surface::getTriangles)
        .method("forEachPoint", &Surface::forEachPoint)
        .method("forEachTriangle", &Surface::forEachTriangle);

    ROSETTA_REGISTER_CLASS(Model)
        // .field("surfaces", &Model::surfaces)
        .property<std::vector<Surface>>("surfaces", &Model::getSurfaces, &Model::setSurfaces)
        .method("getSurfaces",
                &Model::getSurfaces) //- a getter for surfaces (will gen property surfaces)
        .method("setSurfaces",
                &Model::setSurfaces) //- a setter for surfaces (will gen property surfaces)
        .method("addSurface", &Model::addSurface);
}

// ============================================================================

template <typename T> void displayMeta() {
    std::cout << "\n";
    std::cout << typeid(T).name() << " enregistré: " << (ROSETTA_HAS_CLASS(T) ? "Yes" : "No")
              << "\n";
    auto &meta = ROSETTA_GET_META(T);
    std::cout << "Fields:\n";
    for (const auto &field : meta.fields()) {
        std::cout << "  - " << field << "\n";
    }
    std::cout << "Methods:\n";
    for (const auto &meth : meta.methods()) {
        std::cout << "  - " << meth << "\n";
    }
    std::cout << "\n\n";
}

TEST(Introspection2, basic) {
    using namespace rosetta;

    Model model;

    // Create a surface
    std::vector<double> positions = {
        0.0, 0.0, 0.0, // Point 0
        1.0, 0.0, 0.0, // Point 1
        0.0, 1.0, 0.0  // Point 2
    };
    std::vector<int> indices = {
        0, 1, 2 // Triangle using points 0, 1, 2
    };

    Surface surface(positions, indices);
    model.addSurface(surface);

    // Introspect the model
    auto &meta = ROSETTA_GET_META(Model);
    // auto surfaces_any = meta.get_field(model, "surfaces"); // getter/setter not auto detected
    // yet?
    auto        surfaces_any = meta.invoke_method(model, "getSurfaces");
    const auto &surfaces     = surfaces_any.as<std::vector<Surface>>();

    EXPECT_EQ(surfaces.size(), 1);
    const auto &surf = surfaces[0];
    EXPECT_EQ(surf.points.size(), 3);
    EXPECT_EQ(surf.triangles.size(), 1);

    surf.forEachPoint([](const Point &p) {
        // Just print points for debugging
        std::cout << "Point: (" << p.x << ", " << p.y << ", " << p.z << ")\n";
    });
    surf.forEachTriangle([](const Triangle &t) {
        // Just print triangles for debugging
        std::cout << "Triangle: (" << t.a << ", " << t.b << ", " << t.c << ")\n";
    });

    // Check point coordinates
    EXPECT_NEAR(surf.points[1].x, 1.0, 1e-6);
    EXPECT_NEAR(surf.points[2].y, 1.0, 1e-6);

    // Check triangle indices
    EXPECT_EQ(surf.triangles[0].a, 0);
    EXPECT_EQ(surf.triangles[0].b, 1);
    EXPECT_EQ(surf.triangles[0].c, 2);

    displayMeta<Model>();
    displayMeta<Surface>();
    displayMeta<Point>();
    displayMeta<Triangle>();

    meta.dump(std::cout);
}

RUN_TESTS();