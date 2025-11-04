// ============================================================================
// DIAGNOSTIC VERSION - Complete with debug logging
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

    std::cerr << "[DEBUG] Registering classes with Rosetta..." << std::endl;

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
        
    std::cerr << "[DEBUG] Classes registered with Rosetta" << std::endl;
}

// ============================================================================
// Register Class Extractors
// ============================================================================

void register_extractors() {
    using namespace rosetta::generators::python;
    
    std::cerr << "[DEBUG] Registering class extractors..." << std::endl;
    std::cerr << "[DEBUG]   typeid(Vector3D).name() = " << typeid(Vector3D).name() << std::endl;
    
    register_class_extractor<Vector3D>("Vector3D");
    register_class_extractor<Circle>("Circle");
    register_class_extractor<Rectangle>("Rectangle");
    register_class_extractor<Person>("Person");
    
    std::cerr << "[DEBUG] Class extractors registered" << std::endl;
}

// ============================================================================
// Register Class Converters with DEBUG
// ============================================================================

void register_class_converters(rosetta::generators::python::PyGenerator& gen) {
    using namespace rosetta::generators::python;
    
    std::cerr << "[DEBUG] ========================================" << std::endl;
    std::cerr << "[DEBUG] Registering class converters..." << std::endl;
    std::cerr << "[DEBUG] typeid(Vector3D).name() = " << typeid(Vector3D).name() << std::endl;
    
    // Get type index to see what we're registering
    std::type_index vec_idx(typeid(Vector3D));
    std::cerr << "[DEBUG] std::type_index(typeid(Vector3D)).name() = " << vec_idx.name() << std::endl;
    
    // Register converters for Vector3D
    gen.register_converter<Vector3D>(
        // C++ to Python
        [](const rosetta::core::Any& val) -> py::object {
            std::cerr << "[DEBUG] *** Vector3D C++ → Python converter called ***" << std::endl;
            std::cerr << "[DEBUG]   Any.has_value() = " << val.has_value() << std::endl;
            std::cerr << "[DEBUG]   Any.type_name() = " << val.type_name() << std::endl;
            std::cerr << "[DEBUG]   Any.get_type_index().name() = " << val.get_type_index().name() << std::endl;
            
            if (!val.has_value()) {
                std::cerr << "[DEBUG]   Any is empty, returning None" << std::endl;
                return py::none();
            }
            
            try {
                std::cerr << "[DEBUG]   Calling val.as<Vector3D>()..." << std::endl;
                Vector3D vec = val.as<Vector3D>();
                std::cerr << "[DEBUG]   Success! vec = (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
                
                std::cerr << "[DEBUG]   Calling py::cast(vec)..." << std::endl;
                py::object result = py::cast(vec);
                std::cerr << "[DEBUG]   py::cast successful!" << std::endl;
                
                return result;
                
            } catch (const std::bad_cast& e) {
                std::cerr << "[ERROR] bad_cast: " << e.what() << std::endl;
                return py::none();
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] exception: " << e.what() << std::endl;
                return py::none();
            } catch (...) {
                std::cerr << "[ERROR] unknown exception" << std::endl;
                return py::none();
            }
        },
        // Python to C++
        [](const py::object& val) -> rosetta::core::Any {
            std::cerr << "[DEBUG] *** Vector3D Python → C++ converter called ***" << std::endl;
            try {
                Vector3D& vec = val.cast<Vector3D&>();
                std::cerr << "[DEBUG]   Success! vec = (" << vec.x << ", " << vec.y << ", " << vec.z << ")" << std::endl;
                return rosetta::core::Any(vec);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] " << e.what() << std::endl;
                return rosetta::core::Any();
            }
        }
    );
    
    std::cerr << "[DEBUG] Vector3D converter registered!" << std::endl;
    std::cerr << "[DEBUG] Checking if converter was stored..." << std::endl;
    
    // Try to get the converter back
    auto test_converter = gen.get_to_py_converter(std::type_index(typeid(Vector3D)));
    if (test_converter) {
        std::cerr << "[DEBUG] ✓ Converter found in generator!" << std::endl;
    } else {
        std::cerr << "[ERROR] ✗ Converter NOT found in generator!" << std::endl;
    }
    
    std::cerr << "[DEBUG] ========================================" << std::endl;
    
    // Register other converters (without debug for brevity)
    gen.register_converter<Circle>(
        [](const rosetta::core::Any& val) -> py::object {
            try {
                Circle circle = val.as<Circle>();
                return py::cast(circle);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Circle converter: " << e.what() << std::endl;
                return py::none();
            }
        },
        [](const py::object& val) -> rosetta::core::Any {
            try {
                Circle& circle = val.cast<Circle&>();
                return rosetta::core::Any(circle);
            } catch (const std::exception& e) {
                return rosetta::core::Any();
            }
        }
    );
    
    gen.register_converter<Rectangle>(
        [](const rosetta::core::Any& val) -> py::object {
            try {
                Rectangle rect = val.as<Rectangle>();
                return py::cast(rect);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Rectangle converter: " << e.what() << std::endl;
                return py::none();
            }
        },
        [](const py::object& val) -> rosetta::core::Any {
            try {
                Rectangle& rect = val.cast<Rectangle&>();
                return rosetta::core::Any(rect);
            } catch (const std::exception& e) {
                return rosetta::core::Any();
            }
        }
    );
    
    gen.register_converter<Person>(
        [](const rosetta::core::Any& val) -> py::object {
            try {
                Person person = val.as<Person>();
                return py::cast(person);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Person converter: " << e.what() << std::endl;
                return py::none();
            }
        },
        [](const py::object& val) -> rosetta::core::Any {
            try {
                Person& person = val.cast<Person&>();
                return rosetta::core::Any(person);
            } catch (const std::exception& e) {
                return rosetta::core::Any();
            }
        }
    );
    
    std::cerr << "[DEBUG] All class converters registered" << std::endl;
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
// Python Module Definition
// ============================================================================

BEGIN_MODULE(basic, "Example module using PyGenerator - Diagnostic version") {
    std::cerr << "[DEBUG] ========================================"  << std::endl;
    std::cerr << "[DEBUG] MODULE INITIALIZATION STARTING" << std::endl;
    std::cerr << "[DEBUG] ========================================" << std::endl;
    
    // 1. Register classes with Rosetta
    std::cerr << "[DEBUG] Step 1: register_classes()" << std::endl;
    register_classes();
    
    // 2. Register extractors
    std::cerr << "[DEBUG] Step 2: register_extractors()" << std::endl;
    register_extractors();

    // 3. Register utilities
    std::cerr << "[DEBUG] Step 3: REGISTER_UTILITIES()" << std::endl;
    REGISTER_UTILITIES();
    
    std::cerr << "[DEBUG] Step 4: REGISTER_COMMON_CONVERTERS()" << std::endl;
    REGISTER_COMMON_CONVERTERS();
    
    // 4. BIND CLASSES FIRST
    std::cerr << "[DEBUG] Step 5: BIND_CLASSES(...)" << std::endl;
    BIND_CLASSES(Vector3D, Circle, Rectangle, Person);
    std::cerr << "[DEBUG]   Classes bound to pybind11" << std::endl;
    
    // 5. NOW register converters
    std::cerr << "[DEBUG] Step 6: register_class_converters(gen)" << std::endl;
    register_class_converters(gen);
    
    // 6. Bind functions
    std::cerr << "[DEBUG] Step 7: BIND_FUNCTION(...)" << std::endl;
    BIND_FUNCTION(distance, "Calculate distance between two vectors");
    BIND_FUNCTION(create_unit_vector, "Create a normalized vector");
    
    std::cerr << "[DEBUG] ========================================" << std::endl;
    std::cerr << "[DEBUG] MODULE INITIALIZATION COMPLETE" << std::endl;
    std::cerr << "[DEBUG] ========================================" << std::endl;
}
END_MODULE();