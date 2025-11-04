// ============================================================================
// Example: Using PyGenerator to create Python bindings - FIXED VERSION
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

// ============================================================================
// Register Class Extractors (IMPORTANT - THIS IS THE FIX!)
// ============================================================================

void register_extractors() {
    using namespace rosetta::generators::python;

    // Register extractors for each class so py_to_any can properly extract C++ objects
    register_class_extractor<Vector3D>("Vector3D");
    // register_class_extractor<Shape>("Shape");
    register_class_extractor<Circle>("Circle");
    register_class_extractor<Rectangle>("Rectangle");
    register_class_extractor<Person>("Person");
}

void register_class_converters(rosetta::generators::python::PyGenerator &gen) {
    using namespace rosetta::generators::python;

    // Register converters for Vector3D
    gen.register_converter<Vector3D>(
        // C++ to Python - IMPORTANT: Copy the value, don't just take a reference
        [](const rosetta::core::Any &val) -> py::object {
            try {
                // Make a copy to ensure proper object lifetime
                Vector3D vec = val.as<Vector3D>();
                return py::cast(vec);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Vector3D to Python: " << e.what()
                          << std::endl;
                return py::none();
            }
        },
        // Python to C++
        [](const py::object &val) -> rosetta::core::Any {
            try {
                Vector3D &vec = val.cast<Vector3D &>();
                return rosetta::core::Any(vec);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Python to Vector3D: " << e.what()
                          << std::endl;
                return rosetta::core::Any();
            }
        });

    // Register converters for Circle
    gen.register_converter<Circle>(
        [](const rosetta::core::Any &val) -> py::object {
            try {
                Circle circle = val.as<Circle>(); // Copy
                return py::cast(circle);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Circle to Python: " << e.what()
                          << std::endl;
                return py::none();
            }
        },
        [](const py::object &val) -> rosetta::core::Any {
            try {
                Circle &circle = val.cast<Circle &>();
                return rosetta::core::Any(circle);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Python to Circle: " << e.what()
                          << std::endl;
                return rosetta::core::Any();
            }
        });

    // Register converters for Rectangle
    gen.register_converter<Rectangle>(
        [](const rosetta::core::Any &val) -> py::object {
            try {
                Rectangle rect = val.as<Rectangle>(); // Copy
                return py::cast(rect);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Rectangle to Python: " << e.what()
                          << std::endl;
                return py::none();
            }
        },
        [](const py::object &val) -> rosetta::core::Any {
            try {
                Rectangle &rect = val.cast<Rectangle &>();
                return rosetta::core::Any(rect);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Python to Rectangle: " << e.what()
                          << std::endl;
                return rosetta::core::Any();
            }
        });

    // Register converters for Person
    gen.register_converter<Person>(
        [](const rosetta::core::Any &val) -> py::object {
            try {
                Person person = val.as<Person>(); // Copy
                return py::cast(person);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Person to Python: " << e.what()
                          << std::endl;
                return py::none();
            }
        },
        [](const py::object &val) -> rosetta::core::Any {
            try {
                Person &person = val.cast<Person &>();
                return rosetta::core::Any(person);
            } catch (const std::exception &e) {
                std::cerr << "[ERROR] Failed to convert Python to Person: " << e.what()
                          << std::endl;
                return rosetta::core::Any();
            }
        });
}

// ============================================================================
// Free Functions
// ============================================================================

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
    // // Register classes first for Rosetta (introspection)
    // // This will be used for py, js, lua...
    // register_classes();

    // // CRITICAL: Register extractors so C++ objects can be passed between methods
    // register_extractors();

    // REGISTER_UTILITIES();
    // REGISTER_COMMON_CONVERTERS();

    // // BIND CLASSES FIRST - pybind11 needs to know about the types before we can cast to them!
    // BIND_CLASSES(Vector3D, Circle, Rectangle, Person);

    // // NOW register converters (after pybind11 knows about the types)
    // register_class_converters(gen);

    // BIND_FUNCTION(distance, "Calculate distance between two vectors");
    // BIND_FUNCTION(create_unit_vector, "Create a normalized vector");

    register_classes();                                                 // 1. Rosetta metadata
    register_extractors();                                              // 2. Python → C++ extraction
    REGISTER_UTILITIES();                                               // 3. Built-in utilities
    REGISTER_COMMON_CONVERTERS();                                       // 4. STL types

    BIND_CLASSES(Vector3D, Circle, Rectangle, Person);                  // 5. pybind11 bindings (FIRST!)
    register_class_converters(gen);                                     // 6. Converters (AFTER!)

    BIND_FUNCTION(distance, "Calculate distance between two vectors");
    BIND_FUNCTION(create_unit_vector, "Create a normalized vector");    // 7. Free functions
}
END_MODULE();

// PYBIND11_MODULE(basic, m) {
//     m.doc() = "Example module using PyGenerator";

//     // 1. Register classes with Rosetta (for introspection)
//     register_classes();

//     // 2. Register class extractors (IMPORTANT - enables passing C++ objects between methods)
//     register_extractors();

//     // 3. Create generator and bind classes
//     rosetta::generators::python::PyGenerator gen(m);
//     rosetta::generators::python::register_common_converters(gen);

//     // Bind classes
//     gen.bind_classes<Vector3D, Circle, Rectangle, Person>();

//     // Bind free functions
//     gen.bind_function("distance", &distance, "Calculate distance between two vectors");
//     gen.bind_function("create_unit_vector", &create_unit_vector, "Create a normalized vector");

//     // Add utilities
//     gen.add_utilities();
// }