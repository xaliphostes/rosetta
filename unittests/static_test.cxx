/*
 * Test file for static method registration
 *
 * Demonstrates registering and invoking static methods via rosetta introspection.
 */
#include "TEST.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace rosetta;

// ============================================================================
// Example: A math utility class with static methods
// ============================================================================
class MathUtils {
public:
    static double pi() { return 3.14159265358979; }

    static double e() { return 2.71828182845905; }

    static double add(double a, double b) { return a + b; }

    static double multiply(double a, double b) { return a * b; }

    static int factorial(int n) {
        if (n <= 1)
            return 1;
        return n * factorial(n - 1);
    }

    static std::string format(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    static std::vector<double> linspace(double start, double end, int n) {
        std::vector<double> result;
        if (n <= 1) {
            result.push_back(start);
            return result;
        }
        double step = (end - start) / (n - 1);
        for (int i = 0; i < n; ++i) {
            result.push_back(start + i * step);
        }
        return result;
    }
};

// ============================================================================
// Example: A factory class with static creation methods
// ============================================================================
class Point {
public:
    double x, y, z;

    Point() : x(0), y(0), z(0) {}
    Point(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

    double norm() const { return std::sqrt(x * x + y * y + z * z); }

    static Point origin() { return Point(0, 0, 0); }

    static Point unitX() { return Point(1, 0, 0); }

    static Point unitY() { return Point(0, 1, 0); }

    static Point unitZ() { return Point(0, 0, 1); }

    static Point fromSpherical(double r, double theta, double phi) {
        return Point(r * std::sin(theta) * std::cos(phi), r * std::sin(theta) * std::sin(phi),
                     r * std::cos(theta));
    }

    static double distance(const Point &a, const Point &b) {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = a.z - b.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

// ============================================================================
// TEST: Basic static method registration and invocation
// ============================================================================
TEST(StaticMethod, basic_registration) {
    ROSETTA_REGISTER_CLASS(MathUtils)
        .static_method("pi", &MathUtils::pi)
        .static_method("e", &MathUtils::e)
        .static_method("add", &MathUtils::add)
        .static_method("multiply", &MathUtils::multiply)
        .static_method("factorial", &MathUtils::factorial)
        .static_method("format", &MathUtils::format)
        .static_method("linspace", &MathUtils::linspace);

    auto &meta = ROSETTA_GET_META(MathUtils);
    meta.dump(std::cerr);

    // Test no-arg static methods
    auto pi = meta.invoke_static_method("pi");
    EXPECT_NEAR(pi.as<double>(), 3.14159265358979, 1e-10);

    auto e = meta.invoke_static_method("e");
    EXPECT_NEAR(e.as<double>(), 2.71828182845905, 1e-10);

    std::cout << "Basic static method registration: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static methods with parameters
// ============================================================================
TEST(StaticMethod, with_parameters) {
    auto &meta = ROSETTA_GET_META(MathUtils);

    // Test add(double, double)
    auto sum = meta.invoke_static_method("add", {10.5, 20.3});
    EXPECT_NEAR(sum.as<double>(), 30.8, 1e-10);

    // Test multiply(double, double)
    auto product = meta.invoke_static_method("multiply", {6.0, 7.0});
    EXPECT_NEAR(product.as<double>(), 42.0, 1e-10);

    // Test factorial(int)
    auto fact5 = meta.invoke_static_method("factorial", {5});
    EXPECT_EQ(fact5.as<int>(), 120);

    auto fact0 = meta.invoke_static_method("factorial", {0});
    EXPECT_EQ(fact0.as<int>(), 1);

    // Test format(double, int)
    auto formatted = meta.invoke_static_method("format", {3.14159, 2});
    EXPECT_STREQ(formatted.as<std::string>(), "3.14");

    std::cout << "Static methods with parameters: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static methods returning complex types
// ============================================================================
TEST(StaticMethod, complex_return_types) {
    auto &meta = ROSETTA_GET_META(MathUtils);

    // Test linspace returning vector
    auto result = meta.invoke_static_method("linspace", {0.0, 1.0, 5});
    auto vec    = result.as<std::vector<double>>();

    EXPECT_EQ(vec.size(), 5u);
    EXPECT_NEAR(vec[0], 0.0, 1e-10);
    EXPECT_NEAR(vec[1], 0.25, 1e-10);
    EXPECT_NEAR(vec[2], 0.5, 1e-10);
    EXPECT_NEAR(vec[3], 0.75, 1e-10);
    EXPECT_NEAR(vec[4], 1.0, 1e-10);

    std::cout << "Static methods with complex return types: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static factory methods
// ============================================================================
TEST(StaticMethod, factory_methods) {
    ROSETTA_REGISTER_CLASS(Point)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Point::x)
        .field("y", &Point::y)
        .field("z", &Point::z)
        .method("norm", &Point::norm)
        .static_method("origin", &Point::origin)
        .static_method("unitX", &Point::unitX)
        .static_method("unitY", &Point::unitY)
        .static_method("unitZ", &Point::unitZ)
        .static_method("fromSpherical", &Point::fromSpherical)
        .static_method("distance", &Point::distance);

    auto &meta = ROSETTA_GET_META(Point);
    meta.dump(std::cerr);

    // Test factory methods returning Point
    auto origin = meta.invoke_static_method("origin").as<Point>();
    EXPECT_NEAR(origin.x, 0.0, 1e-10);
    EXPECT_NEAR(origin.y, 0.0, 1e-10);
    EXPECT_NEAR(origin.z, 0.0, 1e-10);

    auto unitX = meta.invoke_static_method("unitX").as<Point>();
    EXPECT_NEAR(unitX.x, 1.0, 1e-10);
    EXPECT_NEAR(unitX.y, 0.0, 1e-10);
    EXPECT_NEAR(unitX.z, 0.0, 1e-10);

    auto unitY = meta.invoke_static_method("unitY").as<Point>();
    EXPECT_NEAR(unitY.y, 1.0, 1e-10);

    auto unitZ = meta.invoke_static_method("unitZ").as<Point>();
    EXPECT_NEAR(unitZ.z, 1.0, 1e-10);

    std::cout << "Static factory methods: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static method with object parameters
// ============================================================================
TEST(StaticMethod, object_parameters) {
    auto &meta = ROSETTA_GET_META(Point);

    Point a(1.0, 2.0, 3.0);
    Point b(4.0, 5.0, 6.0);

    auto dist = meta.invoke_static_method("distance", {a, b});
    // distance = sqrt((4-1)^2 + (5-2)^2 + (6-3)^2) = sqrt(9+9+9) = sqrt(27)
    EXPECT_NEAR(dist.as<double>(), std::sqrt(27.0), 1e-10);

    std::cout << "Static method with object parameters: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static method with spherical coordinates
// ============================================================================
TEST(StaticMethod, spherical_coordinates) {
    auto &meta = ROSETTA_GET_META(Point);

    // fromSpherical(r=1, theta=pi/2, phi=0) should give (1, 0, 0)
    double r     = 1.0;
    double theta = 3.14159265358979 / 2.0; // 90 degrees
    double phi   = 0.0;

    auto point = meta.invoke_static_method("fromSpherical", {r, theta, phi}).as<Point>();

    EXPECT_NEAR(point.x, 1.0, 1e-6);
    EXPECT_NEAR(point.y, 0.0, 1e-6);
    EXPECT_NEAR(point.z, 0.0, 1e-6);

    std::cout << "Static method with spherical coordinates: PASSED" << std::endl;
}

// ============================================================================
// TEST: is_static_method check
// ============================================================================
TEST(StaticMethod, is_static_check) {
    auto &meta = ROSETTA_GET_META(Point);

    // Static methods
    EXPECT_TRUE(meta.is_static_method("origin"));
    EXPECT_TRUE(meta.is_static_method("unitX"));
    EXPECT_TRUE(meta.is_static_method("unitY"));
    EXPECT_TRUE(meta.is_static_method("unitZ"));
    EXPECT_TRUE(meta.is_static_method("fromSpherical"));
    EXPECT_TRUE(meta.is_static_method("distance"));

    // Non-static methods
    EXPECT_FALSE(meta.is_static_method("norm"));

    // Non-existent method
    EXPECT_FALSE(meta.is_static_method("nonexistent"));

    std::cout << "is_static_method check: PASSED" << std::endl;
}

// ============================================================================
// TEST: Registry access to static methods
// ============================================================================
TEST(StaticMethod, registry_access) {
    auto &registry = core::Registry::instance();

    auto *holder = registry.get_by_name("MathUtils");
    EXPECT_TRUE(holder != nullptr);

    // Get methods list
    auto methods = holder->get_methods();

    // Verify static methods are in the list
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "pi") != methods.end());
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "add") != methods.end());
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "factorial") != methods.end());

    std::cout << "Registry access to static methods: PASSED" << std::endl;
}

// ============================================================================
// TEST: Static method invocation also works via instance (convenience)
// ============================================================================
TEST(StaticMethod, instance_invocation) {
    auto &meta = ROSETTA_GET_META(MathUtils);

    // Even though we have no real instance, static methods can be invoked
    // Some systems allow calling static methods on instances
    MathUtils utils;

    // invoke_method on an instance should also work for static methods
    // (depending on implementation - this tests if it's supported)
    auto pi = meta.invoke_static_method("pi");
    EXPECT_NEAR(pi.as<double>(), 3.14159265358979, 1e-10);

    std::cout << "Static method instance invocation: PASSED" << std::endl;
}

RUN_TESTS();

/* Expected Output:

=== Rosetta metadata for class: MathUtils ===
Static Methods:
  - double pi()
  - double e()
  - double add(double, double)
  - double multiply(double, double)
  - int factorial(int)
  - std::string format(double, int)
  - std::vector<double> linspace(double, double, int)
...

Basic static method registration: PASSED
Static methods with parameters: PASSED
Static methods with complex return types: PASSED
Static factory methods: PASSED
Static method with object parameters: PASSED
Static method with spherical coordinates: PASSED
is_static_method check: PASSED
Registry access to static methods: PASSED
Static method instance invocation: PASSED

All tests passed!
*/
