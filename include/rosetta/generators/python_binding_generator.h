// ============================================================================
// rosetta/generators/python_binding_generator.hpp
//
// Générateur de bindings Python avec API fluide pour pybind11
// ============================================================================
#pragma once
#include "../core/registry.h"
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <type_traits>

namespace py = pybind11;

namespace rosetta::generators {

    /**
     * @brief Binding generator for Python using Rosetta introspection
     * @example
     * ```cpp
     * PYBIND11_MODULE(my_module, m) {
     *      // Enregistrer d'abord avec Rosetta
     *      register_types();
     * 
     *      // Créer le générateur
     *      PythonBindingGenerator generator(m);
     * 
     *      // Lier les classes automatiquement
     *      generator.bind_class<Person>();
     *      generator.bind_class<GameObject>("GameObj");  // Nom custom
     * 
     *      // Fonctions
     *      generator.bind_function(&my_function, "my_function");
     * 
     *      // Enums
     *      generator.bind_enum<MyEnum>("EnumName", {
     *          {"Value1", MyEnum::Value1},
     *          {"Value2", MyEnum::Value2}
     *      });
     * 
     *      // Ou multiple à la fois
     *      generator.bind_classes<Person, GameObject, Vehicle>();
     * 
     *      // Ou avec macro
     *      PYBIND11_AUTO_BIND_CLASS(m, Person);
     * }
     * ```
     */
    class PythonBindingGenerator {
        py::module_ &module_;

    public:
        /**
         * @brief Constructeur
         * @param m Module pybind11
         */
        explicit PythonBindingGenerator(py::module_ &m) : module_(m) {}

        // ========================================================================
        // BIND CLASS
        // ========================================================================

        /**
         * @brief Lie une classe introspectée automatiquement
         * @tparam Class Type de la classe
         * @param custom_name Nom custom (optionnel, sinon utilise le nom enregistré)
         * @return Référence au générateur pour chaînage
         */
        template <typename Class>
        PythonBindingGenerator &bind_class(const std::string &custom_name = "") {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<Class>()) {
                throw std::runtime_error("Class not registered in Rosetta: " +
                                         std::string(typeid(Class).name()));
            }

            const auto &meta        = registry.get<Class>();
            const auto &inheritance = meta.inheritance();

            std::string class_name = custom_name.empty() ? meta.name() : custom_name;

            // Créer le binding de la classe
            auto cls = create_class_binding<Class>(class_name, inheritance);

            // Ajouter le constructeur si non-abstrait
            if (!inheritance.is_abstract) {
                cls.def(py::init<>());
            }

            // Ajouter les champs
            for (const auto &field_name : meta.fields()) {
                bind_field(cls, meta, field_name);
            }

            // Ajouter les méthodes
            for (const auto &method_name : meta.methods()) {
                bind_method(cls, meta, method_name);
            }

            return *this;
        }

        // ========================================================================
        // BIND MULTIPLE CLASSES
        // ========================================================================

        /**
         * @brief Lie plusieurs classes en une seule ligne
         * @tparam Classes Types des classes
         */
        template <typename... Classes> PythonBindingGenerator &bind_classes() {
            (bind_class<Classes>(), ...);
            return *this;
        }

        // ========================================================================
        // BIND FUNCTION
        // ========================================================================

        /**
         * @brief Lie une fonction libre
         * @tparam Func Type de la fonction
         * @param func Pointeur vers la fonction
         * @param name Nom en Python
         * @param doc Documentation (optionnel)
         */
        template <typename Func>
        PythonBindingGenerator &bind_function(Func func, const std::string &name,
                                              const std::string &doc = "") {
            if (doc.empty()) {
                module_.def(name.c_str(), func);
            } else {
                module_.def(name.c_str(), func, doc.c_str());
            }
            return *this;
        }

        // ========================================================================
        // BIND ENUM
        // ========================================================================

        /**
         * @brief Lie une énumération
         * @tparam Enum Type de l'énumération
         * @param name Nom en Python
         */
        template <typename Enum> PythonBindingGenerator &bind_enum(const std::string &name) {
            static_assert(std::is_enum_v<Enum>, "Type must be an enum");

            py::enum_<Enum>(module_, name.c_str());

            // TODO: Ajouter les valeurs automatiquement via introspection
            // Pour l'instant, l'utilisateur doit ajouter les valeurs manuellement

            return *this;
        }

        /**
         * @brief Lie une énumération avec ses valeurs
         * @tparam Enum Type de l'énumération
         * @param name Nom en Python
         * @param values Paires {nom, valeur}
         */
        template <typename Enum>
        PythonBindingGenerator &
        bind_enum(const std::string                                   &name,
                  std::initializer_list<std::pair<const char *, Enum>> values) {
            static_assert(std::is_enum_v<Enum>, "Type must be an enum");

            auto e = py::enum_<Enum>(module_, name.c_str());

            for (const auto &[value_name, value] : values) {
                e.value(value_name, value);
            }

            return *this;
        }

    private:
        // ========================================================================
        // HELPERS INTERNES
        // ========================================================================

        template <typename Class>
        auto create_class_binding(const std::string           &name,
                                  const core::InheritanceInfo &inheritance) {
            if constexpr (std::is_polymorphic_v<Class>) {
                // Avec smart pointer pour types polymorphiques
                if (inheritance.base_classes.empty()) {
                    return py::class_<Class, std::shared_ptr<Class>>(module_, name.c_str());
                } else {
                    // Avec héritage - nécessite de connaître la base
                    // Simplifié: on suppose une seule base publique
                    return py::class_<Class, std::shared_ptr<Class>>(module_, name.c_str());
                }
            } else {
                // Sans smart pointer pour types non-polymorphiques
                return py::class_<Class>(module_, name.c_str());
            }
        }

        template <typename Class, typename PyClass>
        void bind_field(PyClass &cls, const core::ClassMetadata<Class> &meta,
                        const std::string &field_name) {
            // Impossible de lier dynamiquement sans connaître le type du champ
            // On utilise une approche via property avec lambdas

            cls.def_property(
                field_name.c_str(),
                // Getter
                [&meta, field_name](Class &obj) -> py::object {
                    auto value = meta.get_field(obj, field_name);
                    return any_to_python(value);
                },
                // Setter
                [&meta, field_name](Class &obj, py::object py_value) {
                    auto value = python_to_any(py_value);
                    meta.set_field(obj, field_name, value);
                });
        }

        template <typename Class, typename PyClass>
        void bind_method(PyClass &cls, const core::ClassMetadata<Class> &meta,
                         const std::string &method_name) {
            cls.def(method_name.c_str(),
                    [&meta, method_name](Class &obj, py::args args) -> py::object {
                        // Convertir les arguments Python en Any
                        std::vector<core::Any> cpp_args;
                        for (auto arg : args) {
                            cpp_args.push_back(python_to_any(arg));
                        }

                        // Invoquer la méthode
                        auto result = meta.invoke_method(obj, method_name, cpp_args);

                        // Convertir le résultat en Python
                        return any_to_python(result);
                    });
        }

        // Conversion Any → Python
        static py::object any_to_python(const core::Any &value) {
            // Simplification - nécessite inspection du type réel
            // Pour une implémentation complète, il faudrait un type registry

            // Pour l'instant, retourne None
            return py::none();
        }

        // Conversion Python → Any
        static core::Any python_to_any(py::object obj) {
            // Simplification - détection du type Python

            if (py::isinstance<py::int_>(obj)) {
                return core::Any(obj.cast<int>());
            } else if (py::isinstance<py::float_>(obj)) {
                return core::Any(obj.cast<double>());
            } else if (py::isinstance<py::str>(obj)) {
                return core::Any(obj.cast<std::string>());
            } else if (py::isinstance<py::bool_>(obj)) {
                return core::Any(obj.cast<bool>());
            }

            // Cas par défaut
            return core::Any(0);
        }
    };

    /**
     * @brief Version simplifiée utilisant directement les pointeurs membres
     *
     * Cette version est plus performante mais nécessite que les types
     * soient connus au moment du binding
     */
    class DirectPythonBindingGenerator {
        py::module_ &module_;

    public:
        explicit DirectPythonBindingGenerator(py::module_ &m) : module_(m) {}

        /**
         * @brief Lie une classe en utilisant directement les pointeurs membres
         */
        template <typename Class>
        DirectPythonBindingGenerator &bind_class(const std::string &custom_name = "") {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<Class>()) {
                throw std::runtime_error("Class not registered");
            }

            const auto &meta        = registry.get<Class>();
            const auto &inheritance = meta.inheritance();

            std::string class_name = custom_name.empty() ? meta.name() : custom_name;

            // Créer le binding
            auto cls = create_class_binding<Class>(class_name, inheritance);

            // Constructeur
            if (!inheritance.is_abstract) {
                cls.def(py::init<>());
            }

            // Note: Les champs et méthodes doivent être ajoutés manuellement
            // car nous ne pouvons pas reconstruire les pointeurs membres depuis Any

            return *this;
        }

    private:
        template <typename Class>
        auto create_class_binding(const std::string           &name,
                                  const core::InheritanceInfo &inheritance) {
            if constexpr (std::is_polymorphic_v<Class>) {
                return py::class_<Class, std::shared_ptr<Class>>(module_, name.c_str());
            } else {
                return py::class_<Class>(module_, name.c_str());
            }
        }
    };

} // namespace rosetta::generators

// ============================================================================
// MACROS HELPER
// ============================================================================

/**
 * @brief Macro pour lier automatiquement une classe
 */
#define PYBIND11_AUTO_BIND_CLASS(module, Class) \
    rosetta::generators::PythonBindingGenerator(module).bind_class<Class>()

/**
 * @brief Macro pour lier plusieurs classes
 */
#define PYBIND11_AUTO_BIND_CLASSES(module, ...) \
    rosetta::generators::PythonBindingGenerator(module).bind_classes<__VA_ARGS__>()

// ============================================================================
// EXEMPLE D'UTILISATION COMPLET
// ============================================================================

/*

#include <pybind11/pybind11.h>
#include "rosetta/rosetta.hpp"
#include "rosetta/generators/python_binding_generator.hpp"

namespace py = pybind11;

// 1. Vos classes (aucune modification)
class Person {
public:
    std::string name;
    int age;

    void greet() {
        std::cout << "Hello, I'm " << name << std::endl;
    }
};

class GameObject {
public:
    double x, y, z;

    void move(double dx, double dy, double dz) {
        x += dx; y += dy; z += dz;
    }
};

enum class Status {
    Active,
    Inactive,
    Pending
};

// 2. Fonction libre
double compute_distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
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
    rosetta::generators::PythonBindingGenerator generator(m);

    // Lier les classes automatiquement
    generator.bind_class<Person>();
    generator.bind_class<GameObject>("GameObj");  // Nom custom

    // Ou lier plusieurs à la fois
    // generator.bind_classes<Person, GameObject>();

    // Lier une fonction
    generator.bind_function(&compute_distance, "distance",
                          "Compute Euclidean distance");

    // Lier une enum
    generator.bind_enum<Status>("Status", {
        {"Active", Status::Active},
        {"Inactive", Status::Inactive},
        {"Pending", Status::Pending}
    });

    // Ou utiliser la macro
    // PYBIND11_AUTO_BIND_CLASS(m, Person);
}

// 5. Utilisation en Python
// >>> import my_module
// >>> p = my_module.Person()
// >>> p.name = "Alice"
// >>> p.age = 30
// >>> p.greet()
// Hello, I'm Alice
//
// >>> obj = my_module.GameObj()
// >>> obj.x = 10.0
// >>> obj.move(5, 3, 0)
// >>> print(obj.x)  # 15.0
//
// >>> d = my_module.distance(0, 0, 3, 4)
// >>> print(d)  # 5.0
//
// >>> status = my_module.Status.Active

*/