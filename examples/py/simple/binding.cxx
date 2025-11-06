// ============================================================================
// Example: Using the non-intrusive Python binding generator
// ============================================================================

#include <cmath>
#include <rosetta/generators/py_generator.h>
#include <rosetta/rosetta.h>

#ifndef M_PI
#define M_PI 3.1415926
#endif

// ============================================================================
// Example C++ Classes - NO INHERITANCE REQUIRED!
// ============================================================================

class Vector3D {
public:
    double x, y, z;

    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double length() const { return std::sqrt(x * x + y * y + z * z); }

    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    Vector3D add(const Vector3D &other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    Vector3D scale(double factor) const { return Vector3D(x * factor, y * factor, z * factor); }
};

class Rectangle {
public:
    double width;
    double height;

    Rectangle() : width(0), height(0) {}
    Rectangle(double w, double h) : width(w), height(h) {}

    double area() const { return width * height; }
    double perimeter() const { return 2 * (width + height); }

    bool is_square() const { return std::abs(width - height) < 1e-10; }
};

// Example with properties (getter/setter pattern)
class Person {
private:
    std::string name_;
    int         age_;

public:
    Person() : name_(""), age_(0) {}
    Person(const std::string &n, int a) : name_(n), age_(a) {}

    // Getters
    const std::string &getName() const { return name_; }
    int                getAge() const { return age_; }

    // Setters
    void setName(const std::string &n) { name_ = n; }
    void setAge(const int &a) {
        if (a < 0) {
            throw std::invalid_argument("Age cannot be negative");
        }
        age_ = a;
    }

    std::string greet() const {
        return "Hello, I'm " + name_ + " and I'm " + std::to_string(age_) + " years old";
    }

    void celebrate_birthday() { age_++; }
};

// Example with computed properties
class Circle {
private:
    double radius_;

public:
    Circle() : radius_(0) {}
    Circle(double r) : radius_(r) {
        if (r < 0) {
            throw std::invalid_argument("Radius cannot be negative");
        }
    }

    // Read/write property
    const double &getRadius() const { return radius_; }
    void          setRadius(const double &r) {
        if (r < 0) {
            throw std::invalid_argument("Radius cannot be negative");
        }
        radius_ = r;
    }

    // Read-only computed properties
    double getDiameter() const { return 2 * radius_; }
    double getArea() const { return M_PI * radius_ * radius_; }
    double getCircumference() const { return 2 * M_PI * radius_; }
};

// ============================================================================
// Registration with Rosetta (done once, typically in a registration file)
// ============================================================================

void register_rosetta_classes() {
    // Register Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize)
        .method("add", &Vector3D::add)
        .method("scale", &Vector3D::scale);

    // Register Rectangle with direct fields
    ROSETTA_REGISTER_CLASS(Rectangle)
        .constructor<>()
        .constructor<double, double>()
        .field("width", &Rectangle::width)
        .field("height", &Rectangle::height)
        .method("area", &Rectangle::area)
        .method("perimeter", &Rectangle::perimeter)
        .method("is_square", &Rectangle::is_square);

    // Register Person with properties (demonstrates getter/setter pattern)
    ROSETTA_REGISTER_CLASS(Person)
        .constructor<>()
        .constructor<const std::string &, int>()
        .property<std::string>("name", &Person::getName, &Person::setName)
        .property<int>("age", &Person::getAge, &Person::setAge)
        .method("greet", &Person::greet)
        .method("celebrate_birthday", &Person::celebrate_birthday);

    // Register Circle with properties
    ROSETTA_REGISTER_CLASS(Circle)
        .constructor<>()
        .constructor<double>()
        .property<double>("radius", &Circle::getRadius, &Circle::setRadius)
        .readonly_property<double>("diameter", &Circle::getDiameter)
        .readonly_property<double>("area", &Circle::getArea)
        .readonly_property<double>("circumference", &Circle::getCircumference);
}

// ============================================================================
// Python Module Definition
// ============================================================================

BEGIN_PY_MODULE(rosetta_example, "Python bindings for C++ classes using Rosetta introspection") {
    register_rosetta_classes();
    
    BIND_PY_UTILITIES();
    
    BIND_PY_CLASS(Vector3D);
    BIND_PY_CLASS(Rectangle);
    BIND_PY_CLASS(Person);
    BIND_PY_CLASS(Circle);

    m.attr("PI") = M_PI;

    m.def(
        "create_unit_vector",
        []() {
            Vector3D v(1, 0, 0);
            return v;
        },
        "Create a unit vector along the X axis");
}
END_PY_MODULE()
