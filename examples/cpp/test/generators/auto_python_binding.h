// ============================================================================
// rosetta/generators/auto_python_binding.hpp
//
// Générateur automatique de bindings Python complets depuis le Registry
// ============================================================================
#pragma once
#include "../core/registry.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::generators {

    /**
     * @brief Informations pour générer un binding complet
     */
    struct BindingInfo {
        std::string              class_name;
        std::vector<std::string> fields;
        std::vector<std::string> methods;
        std::vector<std::string> base_classes;
        bool                     is_abstract    = false;
        bool                     is_polymorphic = false;
    };

    /**
     * @brief Collecteur d'informations depuis le Registry
     */
    class BindingCollector {
        std::unordered_map<std::string, BindingInfo> bindings_;

    public:
        /**
         * @brief Collecte les infos d'une classe depuis le Registry
         */
        template <typename Class> void collect() {
            auto &registry = core::Registry::instance();
            if (!registry.has_class<Class>()) {
                return;
            }

            const auto &meta = registry.get<Class>();
            BindingInfo info;

            info.class_name     = meta.name();
            info.fields         = meta.fields();
            info.methods        = meta.methods();
            info.is_abstract    = meta.inheritance().is_abstract;
            info.is_polymorphic = meta.inheritance().is_polymorphic;

            // Collecter les bases
            for (const auto &base : meta.inheritance().base_classes) {
                if (base.access == core::AccessSpecifier::Public) {
                    info.base_classes.push_back(base.name);
                }
            }

            bindings_[info.class_name] = info;
        }

        /**
         * @brief Collecte toutes les classes enregistrées
         */
        void collect_all() {
            // Note: Cette méthode nécessite un moyen d'itérer sur tous les types
            // Pour l'instant, l'utilisateur doit appeler collect<T>() manuellement
        }

        const std::unordered_map<std::string, BindingInfo> &get_bindings() const {
            return bindings_;
        }
    };

    /**
     * @brief Générateur de code Python binding
     */
    class AutoPythonBinding {
        std::string      module_name_;
        BindingCollector collector_;

    public:
        explicit AutoPythonBinding(std::string module_name = "rosetta_module")
            : module_name_(std::move(module_name)) {}

        /**
         * @brief Enregistre une classe pour le binding
         */
        template <typename Class> AutoPythonBinding &add_class() {
            collector_.collect<Class>();
            return *this;
        }

        /**
         * @brief Génère le code complet
         */
        std::string generate() const {
            std::stringstream ss;

            generate_header(ss);
            generate_module_start(ss);
            generate_classes(ss);
            generate_module_end(ss);

            return ss.str();
        }

        /**
         * @brief Génère aussi un fichier CMakeLists.txt
         */
        std::string generate_cmake() const {
            std::stringstream ss;
            ss << "cmake_minimum_required(VERSION 3.15)\n";
            ss << "project(" << module_name_ << "_bindings)\n\n";
            ss << "set(CMAKE_CXX_STANDARD 17)\n\n";
            ss << "find_package(pybind11 REQUIRED)\n\n";
            ss << "pybind11_add_module(" << module_name_ << " bindings.cpp)\n\n";
            ss << "target_include_directories(" << module_name_
               << " PRIVATE ${CMAKE_SOURCE_DIR})\n\n";
            ss << "install(TARGETS " << module_name_ << " DESTINATION .)\n";
            return ss.str();
        }

        /**
         * @brief Génère un exemple Python
         */
        std::string generate_python_example() const {
            std::stringstream ss;
            ss << "#!/usr/bin/env python3\n";
            ss << "# Auto-generated example\n";
            ss << "import " << module_name_ << "\n\n";

            for (const auto &[name, info] : collector_.get_bindings()) {
                if (info.is_abstract)
                    continue;

                ss << "# " << name << "\n";
                ss << "obj = " << module_name_ << "." << name << "()\n";

                if (!info.fields.empty()) {
                    ss << "# Fields: " << info.fields[0];
                    if (info.fields.size() > 1)
                        ss << ", ...";
                    ss << "\n";
                }

                if (!info.methods.empty()) {
                    ss << "# Methods: " << info.methods[0];
                    if (info.methods.size() > 1)
                        ss << ", ...";
                    ss << "\n";
                }

                ss << "\n";
            }

            return ss.str();
        }

    private:
        void generate_header(std::stringstream &ss) const {
            ss << "// Auto-generated by Rosetta AutoPythonBinding\n";
            ss << "// Module: " << module_name_ << "\n";
            ss << "// Do not edit manually!\n\n";
            ss << "#include <pybind11/pybind11.h>\n";
            ss << "#include <pybind11/stl.h>\n";
            ss << "#include <pybind11/functional.h>\n";
            ss << "#include <pybind11/operators.h>\n\n";

            // Inclure les headers des classes
            ss << "// Include your class headers here\n";
            ss << "// #include \"your_classes.hpp\"\n\n";

            ss << "namespace py = pybind11;\n\n";
        }

        void generate_module_start(std::stringstream &ss) const {
            ss << "PYBIND11_MODULE(" << module_name_ << ", m) {\n";
            ss << "    m.doc() = \"Auto-generated Python bindings\";\n\n";
        }

        void generate_classes(std::stringstream &ss) const {
            for (const auto &[name, info] : collector_.get_bindings()) {
                generate_class(ss, info);
            }
        }

        void generate_class(std::stringstream &ss, const BindingInfo &info) const {
            ss << "    // " << info.class_name;
            if (info.is_abstract)
                ss << " (abstract)";
            ss << "\n";

            ss << "    py::class_<" << info.class_name;

            // Bases
            for (const auto &base : info.base_classes) {
                ss << ", " << base;
            }

            // Smart pointer holder si polymorphique
            if (info.is_polymorphic) {
                ss << ", std::shared_ptr<" << info.class_name << ">";
            }

            ss << ">(m, \"" << info.class_name << "\")\n";

            // Constructeur
            if (!info.is_abstract) {
                ss << "        .def(py::init<>())\n";
            }

            // Champs
            for (const auto &field : info.fields) {
                ss << "        .def_readwrite(\"" << field << "\", &" << info.class_name
                   << "::" << field << ")\n";
            }

            // Méthodes
            for (const auto &method : info.methods) {
                ss << "        .def(\"" << method << "\", &" << info.class_name << "::" << method
                   << ")\n";
            }

            ss << "        ;\n\n";
        }

        void generate_module_end(std::stringstream &ss) const { ss << "}\n"; }
    };

/**
 * @brief Macro helper pour enregistrer facilement
 */
#define ROSETTA_PYTHON_BINDING_BEGIN(module_name) \
    rosetta::generators::AutoPythonBinding binding(#module_name);

#define ROSETTA_PYTHON_ADD_CLASS(Class) binding.add_class<Class>();

#define ROSETTA_PYTHON_BINDING_END() std::string binding_code = binding.generate();

} // namespace rosetta::generators

// ============================================================================
// EXEMPLE D'UTILISATION
// ============================================================================

/*

// 1. Définir vos classes
class Vector3D {
public:
    double x, y, z;
    double length() const;
    void normalize();
};

class Shape {
public:
    virtual double area() const = 0;
};

class Circle : public Shape {
public:
    double radius;
    double area() const override;
};

// 2. Enregistrer avec Rosetta
void register_types() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);

    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area");

    ROSETTA_REGISTER_CLASS(Circle)
        .inherits_from<Shape>("Shape")
        .field("radius", &Circle::radius)
        .override_method("area", &Circle::area);
}

// 3. Générer le binding automatiquement
void generate_python_binding() {
    using namespace rosetta::generators;

    AutoPythonBinding binding("my_module");

    binding.add_class<Vector3D>()
           .add_class<Shape>()
           .add_class<Circle>();

    // Générer le code
    std::string cpp_code = binding.generate();
    std::ofstream("bindings.cpp") << cpp_code;

    // Générer CMakeLists.txt
    std::string cmake_code = binding.generate_cmake();
    std::ofstream("CMakeLists.txt") << cmake_code;

    // Générer exemple Python
    std::string py_example = binding.generate_python_example();
    std::ofstream("example.py") << py_example;
}

// 4. Résultat: bindings.cpp contient TOUT automatiquement!
// - Constructeurs
// - Tous les champs (x, y, z, radius)
// - Toutes les méthodes (length, normalize, area)
// - Héritage (Circle hérite de Shape)
// - Classes abstraites gérées

// 5. Compiler et utiliser en Python
// $ mkdir build && cd build
// $ cmake ..
// $ make
// $ python3
// >>> import my_module
// >>> v = my_module.Vector3D()
// >>> v.x = 3.0
// >>> v.y = 4.0
// >>> v.z = 0.0
// >>> v.length()  # 5.0
// >>> v.normalize()
// >>> c = my_module.Circle()
// >>> c.radius = 5.0
// >>> c.area()  # 78.539...

*/