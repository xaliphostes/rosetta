#pragma once
#include "CodeWriter.h"
#include <rosetta/rosetta.h>

// ============================================================================
// TypeScript Declaration Generator (.d.ts) for WASM
// ============================================================================
class TypeScriptGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("// "
             "================================================================="
             "===========");
        line("// AUTO-GENERATED TYPESCRIPT DECLARATIONS - DO NOT EDIT");
        line("// "
             "================================================================="
             "===========");
        line();
        line("export type Vector3 = Float64Array;");
        line("export type Matrix33 = Float64Array;");
        line();

        // Registry should be populated by user's registration function
        // called before the generator runs
        auto &registry = rosetta::core::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            const auto *h = registry.get_by_name(name);
            if (h)
                write_class_decl(name, h);
        }

        write_module_interface();
    }

  private:
    void write_class_decl(const std::string &name,
                          const rosetta::core::Registry::MetadataHolder *h) {
        auto base = h->get_base_class();
        std::string ext;
        if (!base.empty()) {
            size_t pos = base.rfind("::");
            ext = " extends " +
                  (pos != std::string::npos ? base.substr(pos + 2) : base);
        }

        line("export class " + name + ext + " {");
        indent();

        // Write constructors using the new API
        auto ctors = h->get_constructors();
        if (ctors.empty()) {
            line("constructor();");
        } else {
            for (const auto &ctor : ctors) {
                auto params = ctor.get_param_types();
                std::string p;
                for (size_t i = 0; i < params.size(); ++i) {
                    if (i > 0)
                        p += ", ";
                    p +=
                        "arg" + std::to_string(i) + ": " + cpp_to_ts(params[i]);
                }
                line("constructor(" + p + ");");
            }
        }

        // Write method declarations using the new API
        for (const auto &m : h->get_methods()) {
            auto info = h->get_method_info(m);
            auto param_types = info.get_param_types_str();
            auto return_type = info.get_return_type_str();

            std::string p;
            for (size_t i = 0; i < param_types.size(); ++i) {
                if (i > 0)
                    p += ", ";
                p += "arg" + std::to_string(i) + ": " +
                     cpp_to_ts(param_types[i]);
            }
            line(m + "(" + p + "): " + cpp_to_ts(return_type) + ";");
        }

        line("delete(): void;");
        dedent();
        line("}");
        line();
    }

    void write_module_interface() {
        line("export function listClasses(): string[];");
        line("export function getClassMethods(className: string): string[];");
        line();
        line("export interface " + config_.module_name + "Module {");
        indent();

        auto &registry = rosetta::core::Registry::instance();
        for (const auto &name : registry.list_classes()) {
            line(name + ": typeof " + name + ";");
        }
        line("listClasses: typeof listClasses;");
        line("getClassMethods: typeof getClassMethods;");

        dedent();
        line("}");
        line();
        line("declare function create" + config_.module_name +
             "Module(): Promise<" + config_.module_name + "Module>;");
        line("export default create" + config_.module_name + "Module;");
    }

    std::string cpp_to_ts(const std::string &t) {
        if (t == "void")
            return "void";
        if (t == "bool")
            return "boolean";
        if (t == "int" || t == "double" || t == "float" || t == "size_t")
            return "number";
        if (t.find("string") != std::string::npos)
            return "string";
        if (t.find("Vector3") != std::string::npos)
            return "Vector3";
        if (t.find("Matrix33") != std::string::npos)
            return "Matrix33";
        if (t.find("vector<double>") != std::string::npos)
            return "Float64Array";
        if (t.find("vector<int>") != std::string::npos)
            return "Int32Array";

        std::string r = t;
        size_t pos = r.rfind("::");
        if (pos != std::string::npos)
            r = r.substr(pos + 2);
        r.erase(std::remove(r.begin(), r.end(), '*'), r.end());
        r.erase(std::remove(r.begin(), r.end(), '&'), r.end());
        return r.empty() ? "any" : r;
    }
};