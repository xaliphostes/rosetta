#include "TEST.h"

// =============================================================================
// Test class with documentation
// =============================================================================

class Particle {
public:
    double x, y, z;
    double mass;

    Particle() : x(0), y(0), z(0), mass(1.0) {}
    Particle(double x, double y, double z) : x(x), y(y), z(z), mass(1.0) {}
    Particle(double x, double y, double z, double m) : x(x), y(y), z(z), mass(m) {}

    double energy() const { return 0.5 * mass * (x*x + y*y + z*z); }
    void reset() { x = y = z = 0; }

    const double& getMass() const { return mass; }
    void setMass(const double& m) { mass = m; }
};

// Free function for testing
double computeDistance(const Particle& a, const Particle& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

void registerParticleWithDoc() {
    using namespace rosetta;

    ROSETTA_REGISTER_CLASS(Particle)
        .doc("A particle in 3D space with mass")
        .constructor<>().doc("Default constructor: origin with unit mass")
        .constructor<double, double, double>().doc("Constructor with position (unit mass)")
        .constructor<double, double, double, double>().doc("Constructor with position and mass")
        .field("x", &Particle::x).doc("X coordinate")
        .field("y", &Particle::y).doc("Y coordinate")
        .field("z", &Particle::z).doc("Z coordinate")
        .field("mass", &Particle::mass).doc("Particle mass")
        .method("energy", &Particle::energy).doc("Compute kinetic energy")
        .method("reset", &Particle::reset).doc("Reset position to origin")
        .property("massProperty", &Particle::getMass, &Particle::setMass).doc("Mass as property");

    ROSETTA_REGISTER_FUNCTION(computeDistance)
        .doc("Compute Euclidean distance between two particles");
}

// =============================================================================
// Tests
// =============================================================================

TEST(Documentation, ClassDoc) {
    registerParticleWithDoc();

    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    std::string classDoc = holder->get_class_doc();
    EXPECT_EQ(classDoc, "A particle in 3D space with mass");
}

TEST(Documentation, FieldDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    EXPECT_EQ(holder->get_field_doc("x"), "X coordinate");
    EXPECT_EQ(holder->get_field_doc("y"), "Y coordinate");
    EXPECT_EQ(holder->get_field_doc("z"), "Z coordinate");
    EXPECT_EQ(holder->get_field_doc("mass"), "Particle mass");

    // Non-existent field should return empty string
    EXPECT_EQ(holder->get_field_doc("nonexistent"), "");
}

TEST(Documentation, MethodDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    EXPECT_EQ(holder->get_method_doc("energy"), "Compute kinetic energy");
    EXPECT_EQ(holder->get_method_doc("reset"), "Reset position to origin");

    // Non-existent method should return empty string
    EXPECT_EQ(holder->get_method_doc("nonexistent"), "");
}

TEST(Documentation, PropertyDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    EXPECT_EQ(holder->get_property_doc("massProperty"), "Mass as property");

    // Non-existent property should return empty string
    EXPECT_EQ(holder->get_property_doc("nonexistent"), "");
}

TEST(Documentation, ConstructorDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    auto ctors = holder->get_constructors();
    EXPECT_EQ(ctors.size(), 3u);

    // Check constructor docs (order matters)
    EXPECT_EQ(ctors[0].doc, "Default constructor: origin with unit mass");
    EXPECT_EQ(ctors[1].doc, "Constructor with position (unit mass)");
    EXPECT_EQ(ctors[2].doc, "Constructor with position and mass");
}

TEST(Documentation, FunctionDoc) {
    auto& funcRegistry = rosetta::core::FunctionRegistry::instance();
    const auto& funcMeta = funcRegistry.get("computeDistance");
    EXPECT_EQ(funcMeta.doc(), "Compute Euclidean distance between two particles");
}

TEST(Documentation, MethodMetaDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    auto methodMeta = holder->get_method_info("energy");
    EXPECT_EQ(methodMeta.doc, "Compute kinetic energy");
}

TEST(Documentation, PropertyMetaDoc) {
    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("Particle");
    CHECK(holder != nullptr);

    auto propMeta = holder->get_property_info("massProperty");
    EXPECT_EQ(propMeta.doc, "Mass as property");
}

TEST(Documentation, ChainedRegistration) {
    // Test that chained registration still works correctly
    auto& registry = rosetta::Registry::instance();
    auto& metadata = registry.get<Particle>();

    // Verify that all elements were registered correctly
    auto fields = metadata.fields();
    CHECK(std::find(fields.begin(), fields.end(), "x") != fields.end());
    CHECK(std::find(fields.begin(), fields.end(), "y") != fields.end());
    CHECK(std::find(fields.begin(), fields.end(), "z") != fields.end());
    CHECK(std::find(fields.begin(), fields.end(), "mass") != fields.end());

    auto methods = metadata.methods();
    CHECK(std::find(methods.begin(), methods.end(), "energy") != methods.end());
    CHECK(std::find(methods.begin(), methods.end(), "reset") != methods.end());

    auto properties = metadata.properties();
    CHECK(std::find(properties.begin(), properties.end(), "massProperty") != properties.end());
}

TEST(Documentation, EmptyDocByDefault) {
    using namespace rosetta;

    // Register a class without documentation
    class SimpleClass {
    public:
        int value;
        int getValue() const { return value; }
    };

    ROSETTA_REGISTER_CLASS(SimpleClass)
        .constructor<>()
        .field("value", &SimpleClass::value)
        .method("getValue", &SimpleClass::getValue);

    auto& registry = rosetta::Registry::instance();
    auto* holder = registry.get_by_name("SimpleClass");
    CHECK(holder != nullptr);

    // All docs should be empty by default
    EXPECT_EQ(holder->get_class_doc(), "");
    EXPECT_EQ(holder->get_field_doc("value"), "");
    EXPECT_EQ(holder->get_method_doc("getValue"), "");
}

RUN_TESTS();
