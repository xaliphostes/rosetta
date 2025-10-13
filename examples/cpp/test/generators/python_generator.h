// ============================================================================
// rosetta/generators/python_generator.hpp
//
// Générateur de bindings Python avec pybind11
// ============================================================================
#pragma once
#include "../core/registry.h"
#include "binding_generator.h"
#include <sstream>
#include <string>

namespace rosetta::generators {

    /**
     * @brief Générateur de bindings Python utilisant pybind11
     */
    class PythonGenerator : public BindingGenerator {
        std::string module_name_;

    public:
        /**
         * @brief Constructeur
         * @param module_name Nom du module Python (défaut: "rosetta_module")
         */
        explicit PythonGenerator(std::string module_name = "rosetta_module")
            : module_name_(std::move(module_name)) {}

        std::string get_language() const override { return "Python"; }

        std::string get_file_extension() const override { return ".cpp"; }

        std::string generate() const override {
            std::stringstream ss;

            generate_header(ss);
            generate_module_definition(ss);
            generate_classes(ss);
            generate_module_end(ss);

            return ss.str();
        }

        std::string generate_build_instructions() const override {
            std::stringstream ss;
            ss << "# CMakeLists.txt\n";
            ss << "cmake_minimum_required(VERSION 3.15)\n";
            ss << "project(" << module_name_ << ")\n\n";
            ss << "set(CMAKE_CXX_STANDARD 17)\n\n";
            ss << "find_package(pybind11 REQUIRED)\n\n";
            ss << "pybind11_add_module(" << module_name_ << " bindings.cpp)\n\n";
            ss << "# Installation\n";
            ss << "install(TARGETS " << module_name_ << " DESTINATION .)\n";
            return ss.str();
        }

        std::string generate_usage_example() const override {
            std::stringstream ss;
            ss << "# example.py\n";
            ss << "import " << module_name_ << "\n\n";
            ss << "# Créer des instances\n";
            ss << "# obj = " << module_name_ << ".MyClass()\n";
            ss << "# obj.my_field = 42\n";
            ss << "# result = obj.my_method()\n";
            return ss.str();
        }

    private:
        void generate_header(std::stringstream &ss) const {
            ss << "// Auto-generated Python bindings by Rosetta\n";
            ss << "// Target: pybind11\n";
            ss << "// Module: " << module_name_ << "\n\n";
            ss << "#include <pybind11/pybind11.h>\n";
            ss << "#include <pybind11/stl.h>\n";
            ss << "#include <pybind11/functional.h>\n";
            ss << "#include <pybind11/operators.h>\n\n";
            ss << "namespace py = pybind11;\n\n";
        }

        void generate_module_definition(std::stringstream &ss) const {
            ss << "PYBIND11_MODULE(" << module_name_ << ", m) {\n";
            ss << "    m.doc() = \"Auto-generated bindings from Rosetta\";\n\n";
        }

        void generate_classes(std::stringstream &ss) const {
            auto &registry = core::Registry::instance();

            for (const auto &class_name : registry.list_classes()) {
                generate_class(ss, class_name);
            }
        }

        void generate_class(std::stringstream &ss, const std::string &class_name) const {
            auto *holder = core::Registry::instance().get_by_name(class_name);
            if (!holder)
                return;

            const auto &inheritance = holder->get_inheritance();

            ss << "    // " << class_name;
            if (inheritance.is_abstract)
                ss << " (abstract)";
            if (inheritance.is_polymorphic)
                ss << " (polymorphic)";
            ss << "\n";

            // Déclaration de la classe
            ss << "    py::class_<" << class_name;

            // Ajouter les classes de base
            for (const auto &base : inheritance.base_classes) {
                if (base.access == core::AccessSpecifier::Public) {
                    ss << ", " << base.name;
                }
            }

            // Holder pour smart pointer si polymorphique
            if (inheritance.is_polymorphic) {
                ss << ", std::shared_ptr<" << class_name << ">";
            }

            ss << ">(m, \"" << class_name << "\")\n";

            // Constructeur (seulement si non-abstraite)
            if (!inheritance.is_abstract) {
                ss << "        .def(py::init<>())\n";
            }

            // Note: Les champs et méthodes seraient ajoutés ici
            // Mais nécessitent l'accès aux métadonnées typées

            ss << "        ;\n\n";
        }

        void generate_module_end(std::stringstream &ss) const { ss << "}\n"; }
    };

} // namespace rosetta::generators