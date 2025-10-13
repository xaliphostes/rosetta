// ============================================================================
// rosetta/generators/typescript_generator.hpp
//
// Générateur de définitions TypeScript (.d.ts)
// ============================================================================
#pragma once
#include "../core/registry.h"
#include "binding_generator.h"
#include <sstream>
#include <string>

namespace rosetta::generators {

    /**
     * @brief Générateur de fichiers de définition TypeScript
     */
    class TypeScriptGenerator : public BindingGenerator {
        std::string module_name_;

    public:
        /**
         * @brief Constructeur
         * @param module_name Nom du module (défaut: "rosetta_module")
         */
        explicit TypeScriptGenerator(std::string module_name = "rosetta_module")
            : module_name_(std::move(module_name)) {}

        std::string get_language() const override { return "TypeScript"; }

        std::string get_file_extension() const override { return ".d.ts"; }

        std::string generate() const override {
            std::stringstream ss;

            generate_header(ss);
            generate_classes(ss);
            generate_module_export(ss);

            return ss.str();
        }

        std::string generate_build_instructions() const override {
            return "// TypeScript definitions - no compilation needed\n"
                   "// Just include this file in your TypeScript project";
        }

        std::string generate_usage_example() const override {
            std::stringstream ss;
            ss << "// example.ts\n";
            ss << "import * as Module from './" << module_name_ << "';\n\n";
            ss << "// Type-safe usage\n";
            ss << "// const obj = new Module.MyClass();\n";
            ss << "// obj.my_field = 42; // TypeScript checks the type!\n";
            ss << "// const result = obj.my_method();\n";
            return ss.str();
        }

    private:
        void generate_header(std::stringstream &ss) const {
            ss << "// Auto-generated TypeScript definitions by Rosetta\n";
            ss << "// Module: " << module_name_ << "\n\n";
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

            // Commentaire de documentation
            ss << "/**\n";
            ss << " * " << class_name;
            if (inheritance.is_abstract)
                ss << " (abstract class)";
            ss << "\n";
            if (inheritance.is_polymorphic) {
                ss << " * Polymorphic class with virtual methods\n";
            }
            ss << " */\n";

            // Déclaration de la classe
            if (inheritance.is_abstract) {
                ss << "export abstract class " << class_name;
            } else {
                ss << "export class " << class_name;
            }

            // Héritage
            if (!inheritance.base_classes.empty()) {
                const auto &first_base = inheritance.base_classes[0];
                if (first_base.access == core::AccessSpecifier::Public) {
                    ss << " extends " << first_base.name;
                }
            }

            // Interfaces (pour héritage multiple)
            if (inheritance.base_classes.size() > 1) {
                ss << " implements ";
                bool first = true;
                for (size_t i = 1; i < inheritance.base_classes.size(); ++i) {
                    const auto &base = inheritance.base_classes[i];
                    if (base.access == core::AccessSpecifier::Public) {
                        if (!first)
                            ss << ", ";
                        ss << base.name;
                        first = false;
                    }
                }
            }

            ss << " {\n";

            // Constructeur
            if (!inheritance.is_abstract) {
                ss << "    constructor();\n";
            }

            // Note: Les propriétés et méthodes seraient ajoutées ici
            // Nécessite l'accès aux métadonnées typées

            ss << "}\n\n";
        }

        void generate_module_export(std::stringstream &ss) const {
            ss << "// Module exports\n";
            ss << "export as namespace " << module_name_ << ";\n";
        }
    };

} // namespace rosetta::generators