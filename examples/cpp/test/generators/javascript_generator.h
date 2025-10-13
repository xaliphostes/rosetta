// ============================================================================
// rosetta/generators/javascript_generator.hpp
//
// Générateur de bindings JavaScript avec Emscripten
// ============================================================================
#pragma once
#include "../core/registry.h"
#include "binding_generator.h"
#include <sstream>
#include <string>

namespace rosetta::generators {

    /**
     * @brief Générateur de bindings JavaScript utilisant Emscripten
     */
    class JavaScriptGenerator : public BindingGenerator {
        std::string module_name_;

    public:
        /**
         * @brief Constructeur
         * @param module_name Nom du module (défaut: "rosetta_module")
         */
        explicit JavaScriptGenerator(std::string module_name = "rosetta_module")
            : module_name_(std::move(module_name)) {}

        std::string get_language() const override { return "JavaScript"; }

        std::string get_file_extension() const override { return ".cpp"; }

        std::string generate() const override {
            std::stringstream ss;

            generate_header(ss);
            generate_helpers(ss);
            generate_bindings_definition(ss);
            generate_classes(ss);
            generate_bindings_end(ss);

            return ss.str();
        }

        std::string generate_build_instructions() const override {
            std::stringstream ss;
            ss << "# Compilation avec Emscripten\n";
            ss << "emcc bindings.cpp \\\n";
            ss << "  -std=c++17 \\\n";
            ss << "  -lembind \\\n";
            ss << "  -o " << module_name_ << ".js \\\n";
            ss << "  -s MODULARIZE=1 \\\n";
            ss << "  -s EXPORT_NAME=\"" << module_name_ << "\"\n";
            return ss.str();
        }

        std::string generate_usage_example() const override {
            std::stringstream ss;
            ss << "// example.js\n";
            ss << "const Module = require('./" << module_name_ << ".js');\n\n";
            ss << "Module().then(function(instance) {\n";
            ss << "    // Créer des instances\n";
            ss << "    // const obj = new instance.MyClass();\n";
            ss << "    // obj.my_field = 42;\n";
            ss << "    // const result = obj.my_method();\n";
            ss << "});\n";
            return ss.str();
        }

    private:
        void generate_header(std::stringstream &ss) const {
            ss << "// Auto-generated JavaScript bindings by Rosetta\n";
            ss << "// Target: Emscripten/WebAssembly\n";
            ss << "// Module: " << module_name_ << "\n\n";
            ss << "#include <emscripten/bind.h>\n";
            ss << "#include <emscripten/val.h>\n\n";
            ss << "using namespace emscripten;\n\n";
        }

        void generate_helpers(std::stringstream &ss) const {
            ss << "// Helper functions for type conversions\n";
            ss << "template<typename T>\n";
            ss << "val vector_to_array(const std::vector<T>& vec) {\n";
            ss << "    val arr = val::array();\n";
            ss << "    for (size_t i = 0; i < vec.size(); ++i) {\n";
            ss << "        arr.set(i, vec[i]);\n";
            ss << "    }\n";
            ss << "    return arr;\n";
            ss << "}\n\n";
        }

        void generate_bindings_definition(std::stringstream &ss) const {
            ss << "EMSCRIPTEN_BINDINGS(" << module_name_ << ") {\n";

            // Enregistrer les vecteurs standards
            ss << "    // Standard container bindings\n";
            ss << "    register_vector<int>(\"VectorInt\");\n";
            ss << "    register_vector<float>(\"VectorFloat\");\n";
            ss << "    register_vector<double>(\"VectorDouble\");\n";
            ss << "    register_vector<std::string>(\"VectorString\");\n\n";
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
            ss << "    class_<" << class_name;

            // Ajouter les classes de base
            if (!inheritance.base_classes.empty()) {
                ss << ", base<";
                bool first = true;
                for (const auto &base : inheritance.base_classes) {
                    if (base.access == core::AccessSpecifier::Public) {
                        if (!first)
                            ss << ", ";
                        ss << base.name;
                        first = false;
                    }
                }
                ss << ">";
            }

            ss << ">(\"" << class_name << "\")\n";

            // Constructeur (seulement si non-abstraite)
            if (!inheritance.is_abstract) {
                ss << "        .constructor<>()\n";
            }

            // Note: Les propriétés et fonctions seraient ajoutées ici

            ss << "        ;\n\n";
        }

        void generate_bindings_end(std::stringstream &ss) const { ss << "}\n"; }
    };

} // namespace rosetta::generators