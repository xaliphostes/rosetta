// ============================================================================
// Example: Using the non-intrusive JavaScript binding generator
// ============================================================================

#include <rosetta/generators/js/napi_generator.h>
#include <rosetta/rosetta.h>

// ============================================================================
// Example C++ Classes - NO INHERITANCE REQUIRED!
// ============================================================================

class Vector3D {
public:
    double x, y, z;

    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

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
};

class Rectangle {
public:
    double width;
    double height;

    Rectangle() : width(0), height(0) {}
    Rectangle(double w, double h) : width(w), height(h) {}

    double area() const { return width * height; }
    double perimeter() const { return 2 * (width + height); }
};

// Example with properties (getter/setter pattern)
class Person {
private:
    std::string name_;
    int age_;

public:
    Person() : name_(""), age_(0) {}
    Person(const std::string &n, int a) : name_(n), age_(a) {}

    // Getters return by const reference (for string) or by value (for primitives)
    const std::string &getName() const { return name_; }
    int getAge() const { return age_; }

    // Setters take const reference
    void setName(const std::string &n) { name_ = n; }
    void setAge(const int &a) { age_ = a; }

    std::string greet() const {
        return "Hello, I'm " + name_ + " and I'm " + std::to_string(age_) + " years old";
    }
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
        .method("add", &Vector3D::add);

    // Register Rectangle with direct fields
    ROSETTA_REGISTER_CLASS(Rectangle)
        .constructor<>()
        .constructor<double, double>()
        .field("width", &Rectangle::width)
        .field("height", &Rectangle::height)
        .method("area", &Rectangle::area)
        .method("perimeter", &Rectangle::perimeter);

    // Register Person with properties (demonstrates getter/setter pattern)
    ROSETTA_REGISTER_CLASS(Person)
        .constructor<>()
        .constructor<const std::string &, int>()
        .property<std::string>("name", &Person::getName, &Person::setName)
        .property<int>("age", &Person::getAge, &Person::setAge)
        .method("greet", &Person::greet);
}

// ============================================================================
// Node.js Module Initialization
// ============================================================================

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    // Register classes with Rosetta (if not already done)
    register_rosetta_classes();

    // Create the JavaScript binding generator
    rosetta::bindings::JsBindingGenerator generator(env, exports);

    // Bind classes - that's it!
    generator
        .bind_class<Vector3D>()
        .bind_class<Rectangle>()
        .bind_class<Person>()
        .add_utilities();

    return exports;
}

// Register the Node.js module
NODE_API_MODULE(mymodule, InitModule)
