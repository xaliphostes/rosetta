#include "TEST.h"

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
    std::string str() const {
        std::ostringstream os;
        os << "Vector3D(" << x << ", " << y << ", " << z << ")";
        return os.str();
    }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()                       // default ctor
        .constructor<double, double, double>() // parameterized ctor
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}

TEST(Vector3D, basic_test) {
    std::type_index    idx1(typeid(Vector3D));     // Direct type_index
    rosetta::core::Any any_vec(Vector3D(1, 2, 3)); // type_index from Any
    std::type_index    idx2 = any_vec.get_type_index();
    CHECK(idx1 == idx2);

    // Test 4: Extract from Any
    Vector3D vec = any_vec.as<Vector3D>();
    EXPECT_EQ(vec.x, 1);
    EXPECT_EQ(vec.y, 2);
    EXPECT_EQ(vec.z, 3);
}

TEST(Vector3D, introspection) {
    const auto &meta = rosetta::core::Registry::instance().get<Vector3D>(); // should not throw

    // EXPECT_EQ(meta.name(), typeid(Vector3D).name()); // typeid names may differ across compilers
    EXPECT_EQ(meta.fields().size(), 3);
    EXPECT_EQ(meta.methods().size(), 2);
}

TEST(Vector3D, RegistrationAndIntrospection) {
    auto &meta = ROSETTA_GET_META(Vector3D);

    // Check class is instantiable per metadata
    EXPECT_TRUE(meta.is_instantiable()); // :contentReference[oaicite:7]{index=7}

    // Fields present
    const auto &fields = meta.fields();
    EXPECT_TRUE(std::find(fields.begin(), fields.end(), "x") != fields.end());
    EXPECT_TRUE(std::find(fields.begin(), fields.end(), "y") != fields.end());
    EXPECT_TRUE(std::find(fields.begin(), fields.end(), "z") != fields.end());

    // Methods present
    const auto &methods = meta.methods();
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "length") != methods.end());
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "normalize") != methods.end());

    // Optional: show the metadata to the console
    meta.dump(std::cout);
}

TEST(Vector3D, DynamicFieldAccess) {
    auto &meta = ROSETTA_GET_META(Vector3D);

    Vector3D v(3, 4, 0);

    // Read fields dynamically (Any -> double)
    rosetta::Any ax = meta.get_field(v, "x");
    rosetta::Any ay = meta.get_field(v, "y");
    rosetta::Any az = meta.get_field(v, "z");
    EXPECT_EQ(ax.as<double>(), 3.0);
    EXPECT_EQ(ay.as<double>(), 4.0);
    EXPECT_EQ(az.as<double>(), 0.0);

    // Write fields dynamically
    meta.set_field(v, "z", rosetta::Any(12.0));
    EXPECT_EQ(v.z, 12.0);
}

TEST(Vector3D, DynamicMethodInvoke) {
    auto &meta = ROSETTA_GET_META(Vector3D);

    Vector3D v(3, 4, 0);

    // Call length() dynamically (returns Any)
    rosetta::Any len = meta.invoke_method(v, "length");
    EXPECT_NEAR(len.as<double>(), 5.0, 1e-12);

    // Call normalize() dynamically, then check
    meta.invoke_method(v, "normalize");
    EXPECT_NEAR(v.length(), 1.0, 1e-12);

    // For fun, print the object
    std::cout << "After normalize: " << v.str() << " (len=" << v.length() << ")\n";
}

TEST(Vector3D, FieldTypeIntrospection) {
    auto &meta = ROSETTA_GET_META(Vector3D);

    // Verify stored type_info for fields
    auto tx = meta.get_field_type("x");
    auto ty = meta.get_field_type("y");
    auto tz = meta.get_field_type("z");
    EXPECT_TRUE(tx == typeid(double));
    EXPECT_TRUE(ty == typeid(double));
    EXPECT_TRUE(tz == typeid(double));
}

RUN_TESTS();
