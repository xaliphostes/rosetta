#pragma once
#include "../CodeWriter.h"
#include <rosetta/rosetta.h>

// ============================================================================
// Python Type Stub Generator (.pyi)
// ============================================================================
class PythonStubGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("# "
             "================================================================="
             "===========");
        line("# AUTO-GENERATED TYPE STUBS - DO NOT EDIT");
        line("# "
             "================================================================="
             "===========");
        line();
        line("from __future__ import annotations");
        line("from typing import List, Dict, Any, overload");
        line("import numpy as np");
        line("from numpy.typing import NDArray");
        line();
        line("Vector3 = NDArray[np.float64]  # Shape: (3,)");
        line("Matrix33 = NDArray[np.float64]  # Shape: (3, 3)");
        line();

        // Registry should be populated by user's registration function
        // called before the generator runs
        auto &registry = rosetta::core::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            const auto *h = registry.get_by_name(name);
            if (h)
                write_class_stub(name, h);
        }

        write_function_stubs();
    }

  private:
    void write_class_stub(const std::string &name,
                          const rosetta::core::Registry::MetadataHolder *h) {
        auto base = h->get_base_class();
        std::string base_spec;
        if (!base.empty()) {
            size_t pos = base.rfind("::");
            base_spec =
                "(" + (pos != std::string::npos ? base.substr(pos + 2) : base) +
                ")";
        }

        line("class " + name + base_spec + ":");
        indent();
        line("\"\"\"" + name + " - C++ class bound via pybind11\"\"\"");

        // Write constructor stubs using the new API
        auto ctors = h->get_constructors();
        if (ctors.empty()) {
            line("def __init__(self) -> None: ...");
        } else {
            for (const auto &ctor : ctors) {
                auto params = ctor.get_param_types();
                std::string p = "self";
                for (size_t i = 0; i < params.size(); ++i) {
                    p += ", arg" + std::to_string(i) + ": " +
                         cpp_to_py(params[i]);
                }
                line("def __init__(" + p + ") -> None: ...");
            }
        }

        // Write method stubs using the new API
        for (const auto &m : h->get_methods()) {
            auto info = h->get_method_info(m);
            auto param_types = info.get_param_types_str();
            auto return_type = info.get_return_type_str();

            std::string p = "self";
            for (size_t i = 0; i < param_types.size(); ++i) {
                p += ", arg" + std::to_string(i) + ": " +
                     cpp_to_py(param_types[i]);
            }
            line("def " + m + "(" + p + ") -> " + cpp_to_py(return_type) +
                 ": ...");
        }

        line("def __repr__(self) -> str: ...");
        dedent();
        line();
    }

    void write_function_stubs() {
        line("def list_classes() -> List[str]: ...");
        line("def get_class_methods(class_name: str) -> List[str]: ...");
        line("def version() -> str: ...");
    }

    std::string cpp_to_py(const std::string &t) {
        if (t == "void")
            return "None";
        if (t == "bool")
            return "bool";
        if (t == "int" || t == "long" || t == "size_t")
            return "int";
        if (t == "float" || t == "double")
            return "float";
        if (t.find("string") != std::string::npos)
            return "str";
        if (t.find("Vector3") != std::string::npos)
            return "Vector3";
        if (t.find("Matrix33") != std::string::npos)
            return "Matrix33";
        if (t.find("vector<double>") != std::string::npos)
            return "NDArray[np.float64]";
        if (t.find("vector<int>") != std::string::npos)
            return "NDArray[np.int32]";

        std::string r = t;
        size_t pos = r.rfind("::");
        if (pos != std::string::npos)
            r = r.substr(pos + 2);
        r.erase(std::remove(r.begin(), r.end(), '*'), r.end());
        r.erase(std::remove(r.begin(), r.end(), '&'), r.end());
        return r.empty() ? "Any" : r;
    }
};