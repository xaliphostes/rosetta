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

    std::string to_string() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

class Shape {
public:
    std::string name;
    Vector3D    position;

    Shape(const std::string &n = "Shape") : name(n) {}
    virtual ~Shape() = default;

    virtual double      volume() const   = 0;
    virtual std::string get_type() const = 0;
};

class Sphere : public Shape {
public:
    double radius;

    Sphere(double r = 1.0) : Shape("Sphere"), radius(r) {}

    double volume() const override { return (4.0 / 3.0) * M_PI * radius * radius * radius; }

    std::string get_type() const override { return "Sphere"; }
};

class Box : public Shape {
public:
    double width, height, depth;

    Box(double w = 1, double h = 1, double d = 1) : Shape("Box"), width(w), height(h), depth(d) {}

    double volume() const override { return width * height * depth; }

    std::string get_type() const override { return "Box"; }
};

class A {
public:
    std::vector<double>                areas;
    std::vector<Vector3D>              positions;
    std::map<std::string, uint32_t>    map;
    std::array<double, 9>              stress;
    std::vector<std::array<double, 9>> stresses;

    // Functor comme membre
    std::function<double(double, double)>     calculator;
    std::function<void()>                     callback;
    std::function<Vector3D(const Vector3D &)> transformer;

    void setPositions(const std::vector<Vector3D> &pos) { positions = pos; }
    void setAreas(const std::vector<double> &as) { areas = as; }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize)
        .method("to_string", &Vector3D::to_string);

    // Shape (abstract)
    ROSETTA_REGISTER_CLASS(Shape)
        .field("name", &Shape::name)
        .field("position", &Shape::position)
        .pure_virtual_method<double>("volume")
        .pure_virtual_method<std::string>("get_type");

    // Sphere
    ROSETTA_REGISTER_CLASS(Sphere)
        .inherits_from<Shape>("Shape")
        .field("radius", &Sphere::radius)
        .base_field<Shape>("name", &Shape::name)
        .base_field<Shape>("position", &Shape::position)
        .override_method("volume", &Sphere::volume)
        .override_method("get_type", &Sphere::get_type);

    // Box
    ROSETTA_REGISTER_CLASS(Box)
        .inherits_from<Shape>("Shape")
        .field("width", &Box::width)
        .field("height", &Box::height)
        .field("depth", &Box::depth)
        .base_field<Shape>("name", &Shape::name)
        .base_field<Shape>("position", &Shape::position)
        .override_method("volume", &Box::volume)
        .override_method("get_type", &Box::get_type);

    // A
    ROSETTA_REGISTER_CLASS(A)
        .field("positions", &A::positions)
        .field("areas", &A::areas)
        .field("map", &A::map)
        .field("stress", &A::stress)
        .field("stresses", &A::stresses)
        .field("calculator", &A::calculator)
        .field("callback", &A::callback)
        .field("transformer", &A::transformer)

        .method("setAreas", &A::setAreas)
        .method("setPositions", &A::setPositions);
}

// ============================================================================

TEST(Introspection, demo) {
    Vector3D vec(3, 4, 0);
    auto    &meta = ROSETTA_GET_META(Vector3D);

    std::cout << "\nVector3D initial: " << vec.to_string() << "\n";
    std::cout << "Champs enregistrés: ";
    for (const auto &field : meta.fields()) {
        std::cout << field << " ";
    }
    std::cout << "\n";

    // Accès dynamique
    auto x_value = meta.get_field(vec, "x");
    std::cout << "Valeur de x (dynamique): " << x_value.as<double>() << "\n";
    EXPECT_EQ(x_value.as<double>(), 3.0);

    // Modification dynamique
    meta.set_field(vec, "x", rosetta::Any(10.0));
    std::cout << "Après modification: " << vec.to_string() << "\n";
    EXPECT_NEAR(meta.get_field(vec, "x").as<double>(), 10.0, 1e-4);

    // Appel de méthode
    auto length = meta.invoke_method(vec, "length");
    std::cout << "Longueur: " << length.as<double>() << "\n";
    EXPECT_NEAR(length.as<double>(), 10.7703, 1e-4);

    meta.invoke_method(vec, "normalize");
    std::cout << "Après normalisation: " << vec.to_string() << "\n";
    EXPECT_NEAR(vec.length(), 1, 1e-4);
}

TEST(Introspection, A) {
    A a;
    a.areas     = {1, 2, 3, 4, 5, 6};
    a.positions = {{1, 2, 3}, {4, 5, 6}};

    a.calculator  = [](double x, double y) { return x + y; };
    a.callback    = []() { std::cout << "Hello!\n"; };
    a.transformer = [](const Vector3D &v) { return Vector3D(v.x * 2, v.y * 2, v.z * 2); };

    auto &meta = ROSETTA_GET_META(A);
    std::cout << "\n";

    // Accès dynamique
    {
        auto        x_value = meta.get_field(a, "areas");
        const auto &v       = x_value.as<std::vector<double>>();
        EXPECT_ARRAY_NEAR(v, std::vector<double>({1, 2, 3, 4, 5, 6}), 1e-6);
    }
    {
        auto        x_value = meta.get_field(a, "positions");
        const auto &v       = x_value.as<std::vector<Vector3D>>();
        auto        result  = std::vector<Vector3D>({Vector3D(1, 2, 3), Vector3D(4, 5, 6)});
        int         i       = 0;
        for (auto m : v) {
            EXPECT_NEAR(m.x, result[i].x, 1e-6);
            EXPECT_NEAR(m.y, result[i].y, 1e-6);
            EXPECT_NEAR(m.z, result[i].z, 1e-6);
            ++i;
        }
    }

    // Functors
    EXPECT_NEAR(a.calculator(10, 20), 30, 1e-6);
    a.callback();
    auto v2 = a.transformer(Vector3D(1, 2, 3));
    EXPECT_NEAR(v2.x, 2, 1e-6);
    EXPECT_NEAR(v2.y, 4, 1e-6);
    EXPECT_NEAR(v2.z, 6, 1e-6);
}

TEST(Introspection, inheritance) {
    auto       &sphere_meta = ROSETTA_GET_META(Sphere);
    const auto &inheritance = sphere_meta.inheritance();

    // std::cout << "\nSphere:\n";
    // std::cout << "  Est abstraite: " << inheritance.is_abstract << "\n";
    // std::cout << "  Est polymorphique: " << inheritance.is_polymorphic << "\n";
    // std::cout << "  Nombre de classes de base: " << inheritance.base_classes.size() << "\n";

    EXPECT_EQ(inheritance.is_abstract, false);
    EXPECT_EQ(inheritance.is_polymorphic, true);
    EXPECT_EQ(inheritance.base_classes.size(), 1);
    CHECK(inheritance.base_classes.size() == 1);
    EXPECT_STREQ(inheritance.base_classes[0].name.c_str(), "Shape");

    // Test polymorphisme
    Sphere sphere(5.0);
    sphere.name = "Big Sphere";
    auto vol    = sphere_meta.invoke_method(sphere, "volume");
    EXPECT_NEAR(vol.as<double>(), (4.0 / 3.0) * M_PI * 125.0, 1e-4);
}

TEST(Introspection, serialization) {
    Vector3D vec(1.5, 2.5, 3.5);

    // JSON
    {
        std::string json = rosetta::JSONSerializer::serialize(vec);
        EXPECT_STREQ(json.c_str(), "{\n"
                                   "  \"x\": \"value\",\n"
                                   "  \"y\": \"value\",\n"
                                   "  \"z\": \"value\"\n"
                                   "}");
    }

    // XML
    std::string xml = rosetta::XMLSerializer::serialize(vec, "Vector3D");
    EXPECT_STREQ(xml.c_str(), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                              "<Vector3D type=\"Vector3D\">\n"
                              "  <x>value</x>\n"
                              "  <y>value</y>\n"
                              "  <z>value</z>\n"
                              "</Vector3D>\n");

    A a;
    {
        std::string json = rosetta::JSONSerializer::serialize(a);

        // TODO: have better serialization for complex types

        EXPECT_STREQ(json.c_str(), "{\n"
                                   "  \"positions\": \"value\",\n"
                                   "  \"areas\": \"value\",\n"
                                   "  \"map\": \"value\",\n"
                                   "  \"stress\": \"value\",\n"
                                   "  \"stresses\": \"value\",\n"
                                   "  \"calculator\": \"value\",\n"
                                   "  \"callback\": \"value\",\n"
                                   "  \"transformer\": \"value\"\n"
                                   "}");
    }
}

TEST(Introspection, validation) {
    using namespace rosetta;

    // Ajouter des contraintes
    ConstraintValidator::instance().add_field_constraint<Sphere, double>(
        "radius", make_range_constraint(0.1, 100.0));

    // Test avec valeur valide
    Sphere                   valid_sphere(5.0);
    std::vector<std::string> errors;

    CHECK(ConstraintValidator::instance().validate(valid_sphere, errors) == true);

    // Test avec valeur invalide
    Sphere invalid_sphere(-5.0);
    errors.clear();

    CHECK(ConstraintValidator::instance().validate(invalid_sphere, errors) == false);
    EXPECT_STREQ(errors[0].c_str(), "radius: Value must be between 0.100000 and 100.000000");
}

TEST(Introspection, documentation) {
    // Markdown
    std::cout << "\n--- Markdown ---\n";
    rosetta::DocGenerator md_gen(rosetta::DocFormat::Markdown);
    std::string           markdown = md_gen.generate();
    std::cout << markdown.substr(0, 800) << "...\n";
}

template <typename T> void displayMeta() {
    std::cout << "\n";
    std::cout << typeid(T).name() << " enregistré: " << (ROSETTA_HAS_CLASS(Vector3D) ? "Yes" : "No")
              << "\n";
    auto &meta = ROSETTA_GET_META(T);
    std::cout << "Fields:\n";
    for (const auto &field : meta.fields()) {
        std::cout << "  - " << field << "\n";
    }
    std::cout << "Methods:\n";
    for (const auto &meth : meta.methods()) {
        std::cout << "  - " << meth << "\n";
    }
    std::cout << "\n\n";
}

TEST(Introspection, registry) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "DEMO 7: REGISTRY\n";
    std::cout << std::string(60, '=') << "\n";

    auto &registry = rosetta::Registry::instance();

    std::cout << "\nClasses enregistrées: " << registry.size() << "\n";
    std::cout << "Liste:\n";
    for (const auto &name : registry.list_classes()) {
        std::cout << "  - " << name << "\n";
    }

    std::cout << "\nVérifications:\n";
    displayMeta<Vector3D>();
    displayMeta<Shape>();
    displayMeta<Sphere>();
    displayMeta<Box>();
    displayMeta<A>();
}

RUN_TESTS();