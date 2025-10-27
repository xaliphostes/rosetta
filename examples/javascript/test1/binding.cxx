// ============================================================================
// Example: Runtime N-API Binding Generator
// ============================================================================

#include <rosetta/rosetta.h>
#include <rosetta/generators/js/BindingGenerator.h>
#include <rosetta/generators/js/TypeConverterRegistry.h>
#include <cmath>
#include <iostream>

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

    Vector3D scale(double factor) const { return Vector3D(x * factor, y * factor, z * factor); }

    std::string to_string() const {
        return "Vector3D(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
               std::to_string(z) + ")";
    }
};

class Shape {
public:
    virtual ~Shape()                      = default;
    virtual double      area() const      = 0;
    virtual double      perimeter() const = 0;
    virtual std::string type() const      = 0;
};

class Circle : public Shape {
public:
    double radius;

    explicit Circle(double r = 1.0) : radius(r) {}

    double area() const override { return 3.14159 * radius * radius; }

    double perimeter() const override { return 2.0 * 3.14159 * radius; }

    std::string type() const override { return "Circle"; }
};

class Rectangle : public Shape {
public:
    double width, height;

    Rectangle(double w = 1.0, double h = 1.0) : width(w), height(h) {}

    double area() const override { return width * height; }

    double perimeter() const override { return 2.0 * (width + height); }

    std::string type() const override { return "Rectangle"; }
};

// ============================================================================
// Example Enum
// ============================================================================

enum class Color { Red = 0, Green = 1, Blue = 2, Yellow = 3, Magenta = 4, Cyan = 5 };

enum class ShapeType { Circle = 0, Rectangle = 1, Triangle = 2, Polygon = 3 };

// ============================================================================
// Global Functions
// ============================================================================

double calculate_distance(const Vector3D &a, const Vector3D &b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::vector<double> generate_sequence(int count, double start, double step) {
    std::vector<double> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(start + i * step);
    }
    return result;
}

std::map<std::string, double> get_statistics(const std::vector<double> &data) {
    if (data.empty()) {
        return {};
    }

    double sum = 0.0;
    double min = data[0];
    double max = data[0];

    for (double val : data) {
        sum += val;
        if (val < min)
            min = val;
        if (val > max)
            max = val;
    }

    double mean = sum / data.size();

    return {{"min", min},
            {"max", max},
            {"mean", mean},
            {"sum", sum},
            {"count", static_cast<double>(data.size())}};
}

void apply_function(const std::vector<double> &values, std::function<void(double)> callback) {
    for (double val : values) {
        callback(val);
    }
}

std::vector<double> transform_values(const std::vector<double>    &values,
                                     std::function<double(double)> transformer) {
    std::vector<double> result;
    result.reserve(values.size());
    for (double val : values) {
        result.push_back(transformer(val));
    }
    return result;
}

// ============================================================================
// Rosetta Registration
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
        .method("scale", &Vector3D::scale)
        .method("to_string", &Vector3D::to_string);

    // Register Shape (abstract base)
    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area")
        .pure_virtual_method<double>("perimeter")
        .pure_virtual_method<std::string>("type");

    // Register Circle
    ROSETTA_REGISTER_CLASS(Circle)
        .inherits_from<Shape>("Shape")
        .field("radius", &Circle::radius)
        .override_method("area", &Circle::area)
        .override_method("perimeter", &Circle::perimeter)
        .override_method("type", &Circle::type);

    // Register Rectangle
    ROSETTA_REGISTER_CLASS(Rectangle)
        .inherits_from<Shape>("Shape")
        .field("width", &Rectangle::width)
        .field("height", &Rectangle::height)
        .override_method("area", &Rectangle::area)
        .override_method("perimeter", &Rectangle::perimeter)
        .override_method("type", &Rectangle::type);
}

void register_type_converters() {
    using namespace rosetta::generators::js;

    auto &registry = TypeConverterRegistry::instance();

    // Containers
    registry.register_container_if_needed<std::vector<double>>();
    registry.register_container_if_needed<std::map<std::string, double>>();

    // Functions/callbacks
    registry.register_converter<std::function<void(double)>>(
        std::make_unique<FunctionConverter<void, double>>());

    registry.register_converter<std::function<double(double)>>(
        std::make_unique<FunctionConverter<double, double>>());
}

// ============================================================================
// Module Initialization
// ============================================================================

BEGIN_NAPI_MODULE(geometry) {
    using namespace rosetta::generators::js;

    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║      Using Rosetta Runtime N-API Binding Generator     ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";

    register_classes();
    register_type_converters();

    BindingGenerator generator(env, exports);

    generator.bind_classes<Vector3D, Circle, Rectangle>();

    TypeConverterRegistry::instance().register_class_wrapper<Vector3D>("Vector3D");

    generator.bind_function(calculate_distance, "calculateDistance")
        .bind_function(generate_sequence, "generateSequence")
        .bind_function(get_statistics, "getStatistics")
        .bind_function(apply_function, "applyFunction")
        .bind_function(transform_values, "transformValues");

    generator.bind_enum<Color>("Color", {{"Red", Color::Red},
                                         {"Green", Color::Green},
                                         {"Blue", Color::Blue},
                                         {"Yellow", Color::Yellow},
                                         {"Magenta", Color::Magenta},
                                         {"Cyan", Color::Cyan}});

    generator.bind_enum<ShapeType>("ShapeType", {{"Circle", ShapeType::Circle},
                                                 {"Rectangle", ShapeType::Rectangle},
                                                 {"Triangle", ShapeType::Triangle},
                                                 {"Polygon", ShapeType::Polygon}});

    return exports;
}

END_NAPI_MODULE(geometry)
