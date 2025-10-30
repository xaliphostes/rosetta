#include <rosetta/rosetta.h>

// pybind11 module definition using automatic binding
PYBIND11_MODULE(basic, m) {
    m.doc() = "Automatic Python bindings using C++ introspection";

    rosetta::PythonBindingGenerator(m);
    generator.bind_classes<Person, Vehicle>();
}

#include "rosetta/rosetta.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

// 1. Vos classes (aucune modification)
class Person {
public:
    std::string name;
    int         age;

    void greet() { std::cout << "Hello, I'm " << name << std::endl; }
};

class GameObject {
public:
    double x, y, z;

    void move(double dx, double dy, double dz) {
        x += dx;
        y += dy;
        z += dz;
    }
};

enum class Status { Active, Inactive, Pending };

// 2. Fonction libre
double compute_distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// 3. Enregistrement Rosetta
void register_types() {
    ROSETTA_REGISTER_CLASS(Person)
        .field("name", &Person::name)
        .field("age", &Person::age)
        .method("greet", &Person::greet);

    ROSETTA_REGISTER_CLASS(GameObject)
        .field("x", &GameObject::x)
        .field("y", &GameObject::y)
        .field("z", &GameObject::z)
        .method("move", &GameObject::move);
}

// 4. Module Python avec binding automatique
PYBIND11_MODULE(my_module, m) {
    m.doc() = "My awesome module with automatic bindings";

    // Enregistrer les types d'abord
    register_types();

    // Créer le générateur
    rosetta::PythonBindingGenerator generator(m);

    // Lier les classes automatiquement
    generator.bind_class<Person>();
    generator.bind_class<GameObject>("GameObj"); // Nom custom

    // Ou lier plusieurs à la fois
    // generator.bind_classes<Person, GameObject>();

    // Lier une fonction
    generator.bind_function(&compute_distance, "distance", "Compute Euclidean distance");

    // Lier une enum
    generator.bind_enum<Status>(
        "Status",
        {{"Active", Status::Active}, {"Inactive", Status::Inactive}, {"Pending", Status::Pending}});

    // Ou utiliser la macro
    // PYBIND11_AUTO_BIND_CLASS(m, Person);
}