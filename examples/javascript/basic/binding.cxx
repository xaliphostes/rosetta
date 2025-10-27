#include <node_api.h>
#include <rosetta/generators/javascript_napi_binding_generator.h>
#include <rosetta/rosetta.h>

// 1. Vos classes (aucune modification)
class Vector3D {
public:
    double x, y, z;

    Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    double length() const { return std::sqrt(x * x + y * y + z * z); }

    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
};

class Point2D {
public:
    double x, y;

    Point2D(double x = 0, double y = 0) : x(x), y(y) {}

    double distance() const { return std::sqrt(x * x + y * y); }
};

class A {
public:
    std::vector<double>                areas;
    std::vector<Vector3D>              positions;
    std::map<std::string, uint32_t>    map;
    std::array<double, 9>              stress;
    std::vector<std::array<double, 9>> stresses;

    void setPositions(const std::vector<Vector3D> &pos) { positions = pos; }
    void setAreas(const std::vector<double> &as) { areas = as; }
};

// enum class Status { Active, Inactive, Pending };

// 2. Enregistrement Rosetta
void register_types() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);

    ROSETTA_REGISTER_CLASS(Point2D)
        .field("x", &Point2D::x)
        .field("y", &Point2D::y)
        .method("distance", &Point2D::distance);

    ROSETTA_REGISTER_CLASS(A)
        .field("positions", &A::positions)
        .field("areas", &A::areas)
        .field("map", &A::map)
        .field("stress", &A::stress)
        .field("stresses", &A::stresses)
        .method("setAreas", &A::setAreas)
        .method("setPositions", &A::setPositions);
}

// 3. Module Node.js avec binding automatique
BEGIN_MODULE(basic) {
    register_types();

    rosetta::generators::NapiBindingGenerator(env, exports)
        .bind_class<Vector3D>()
        .bind_class<Point2D>("Point") // Custom name
        .bind_class<A>();

    return exports;
}
END_MODULE(basic)