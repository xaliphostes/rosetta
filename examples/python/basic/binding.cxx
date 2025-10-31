// ============================================================================
// Example: Using PyGenerator to create Python bindings
// ============================================================================

#include <iostream>
#include <rosetta/generators/py/py_generator.h>
#include <rosetta/generators/py/type_converters.h>
#include <vector>

// ============================================================================
// Example Classes
// ============================================================================

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

    Vector3D add(const Vector3D &other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    std::string to_string() const {
        return "Vector3D(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
               std::to_string(z) + ")";
    }
};

class Shape {
public:
    virtual ~Shape()                 = default;
    virtual double      area() const = 0;
    virtual std::string name() const = 0;
};

class Circle : public Shape {
public:
    double radius;

    Circle(double r = 1.0) : radius(r) {}

    double area() const override { return 3.14159 * radius * radius; }

    std::string name() const override { return "Circle"; }
};

class Rectangle : public Shape {
public:
    double width, height;

    Rectangle(double w = 1.0, double h = 1.0) : width(w), height(h) {}

    double area() const override { return width * height; }

    std::string name() const override { return "Rectangle"; }
};

class Person {
public:
    std::string              name;
    int                      age;
    std::vector<std::string> hobbies;

    Person(std::string n = "", int a = 0) : name(std::move(n)), age(a) {}

    void add_hobby(const std::string &hobby) { hobbies.push_back(hobby); }

    std::vector<std::string> get_hobbies() const { return hobbies; }

    std::string introduce() const {
        return "Hi, I'm " + name + " and I'm " + std::to_string(age) + " years old.";
    }
};

// ============================================================================
// Register Classes with Rosetta
// ============================================================================

void register_classes() {
    using namespace rosetta;

    // Register Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize)
        .method("add", &Vector3D::add)
        .method("to_string", &Vector3D::to_string);

    // Register Shape (abstract base)
    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area")
        .pure_virtual_method<std::string>("name");

    // Register Circle
    ROSETTA_REGISTER_CLASS(Circle)
        .inherits_from<Shape>("Shape")
        .field("radius", &Circle::radius)
        .override_method("area", &Circle::area)
        .override_method("name", &Circle::name);

    // Register Rectangle
    ROSETTA_REGISTER_CLASS(Rectangle)
        .inherits_from<Shape>("Shape")
        .field("width", &Rectangle::width)
        .field("height", &Rectangle::height)
        .override_method("area", &Rectangle::area)
        .override_method("name", &Rectangle::name);

    // Register Person
    ROSETTA_REGISTER_CLASS(Person)
        .field("name", &Person::name)
        .field("age", &Person::age)
        .field("hobbies", &Person::hobbies)
        .method("add_hobby", &Person::add_hobby)
        .method("get_hobbies", &Person::get_hobbies)
        .method("introduce", &Person::introduce);
}

double distance(const Vector3D &a, const Vector3D &b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3D create_unit_vector(double x, double y, double z) {
    Vector3D v(x, y, z);
    v.normalize();
    return v;
}

// ============================================================================
// Python Module Definition - Example 1: Manual binding
// ============================================================================

BEGIN_MODULE(basic, "Example module using PyGenerator - Manual approach") {
    // Register classes first for Rosetta (introspection)
    // This will be used for py, js, lua...
    register_classes();

    REGISTER_UTILITIES();
    REGISTER_COMMON_CONVERTERS();

    BIND_CLASSES(Vector3D, Circle, Rectangle, Person);
    
    BIND_FUNCTION(distance, "Calculate distance between two vectors");
    BIND_FUNCTION(create_unit_vector, "Create a normalized vector");
}
END_MODULE();

// ============================================================================
// Python Module Definition - Example 3: With custom functions
// ============================================================================

/*
// Free function to bind
double distance(const Vector3D &a, const Vector3D &b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3D create_unit_vector(double x, double y, double z) {
    Vector3D v(x, y, z);
    v.normalize();
    return v;
}

PYBIND11_MODULE(basic, m) {
    m.doc() = "Example with custom functions";

    register_classes();

    rosetta::generators::python::PyGenerator gen(m);
    rosetta::generators::python::register_common_converters(gen);

    // Bind classes
    gen.bind_classes<Vector3D, Circle, Rectangle, Person>();

    // Bind free functions
    gen.bind_function("distance", &distance, "Calculate distance between two vectors");
    gen.bind_function("create_unit_vector", &create_unit_vector, "Create a normalized vector");

    gen.add_utilities();
}
*/

// ============================================================================
// Python Module Definition - Example 4: With custom converters
// ============================================================================

/*
struct Color {
    unsigned char r, g, b, a;
    Color(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255)
        : r(r), g(g), b(b), a(a) {}
};

void register_color_class() {
    ROSETTA_REGISTER_CLASS(Color)
        .field("r", &Color::r)
        .field("g", &Color::g)
        .field("b", &Color::b)
        .field("a", &Color::a);
}

PYBIND11_MODULE(basic, m) {
    m.doc() = "Example with custom type converter";

    register_color_class();

    rosetta::generators::python::PyGenerator gen(m);

    // Register custom converter for Color
    gen.register_converter<Color>(
        // C++ to Python: return as tuple (r, g, b, a)
        [](const rosetta::core::Any &val) -> py::object {
            const Color &c = val.as<Color>();
            return py::make_tuple(c.r, c.g, c.b, c.a);
        },
        // Python to C++: accept tuple or list
        [](const py::object &val) -> rosetta::core::Any {
            if (PyTuple_Check(val.ptr()) || PyList_Check(val.ptr())) {
                auto seq = val.cast<py::sequence>();
                if (seq.size() >= 3) {
                    unsigned char r = seq[0].cast<unsigned char>();
                    unsigned char g = seq[1].cast<unsigned char>();
                    unsigned char b = seq[2].cast<unsigned char>();
                    unsigned char a = seq.size() >= 4 ? seq[3].cast<unsigned char>() : 255;
                    return rosetta::core::Any(Color(r, g, b, a));
                }
            }
            return rosetta::core::Any(Color());
        });

    gen.bind_class<Color>("Color");
}
*/
