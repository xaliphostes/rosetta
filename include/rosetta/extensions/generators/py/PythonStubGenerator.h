#pragma once
#include "../common/CodeWriter.h"
#include <rosetta/rosetta.h>
#include <rosetta/core/function_registry.h>

// ============================================================================
// Python Type Stub Generator (.pyi) - Modernized
// ============================================================================
class PythonStubGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        write_header();
        write_class_stubs();
        write_free_function_stubs();
        write_utility_function_stubs();
    }

private:
    void write_header() {
        emit(R"(
# ==============================================================================
# AUTO-GENERATED TYPE STUBS - DO NOT EDIT
# ==============================================================================

from __future__ import annotations
from typing import List, Dict, Any, overload, Callable
import numpy as np
from numpy.typing import NDArray

Vector3 = NDArray[np.float64]  # Shape: (3,)
Matrix33 = NDArray[np.float64]  # Shape: (3, 3)
)");
    }

    void write_class_stubs() {
        auto &registry = rosetta::core::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            const auto *h = registry.get_by_name(name);
            if (h) write_class_stub(name, h);
        }
    }

    void write_class_stub(const std::string &name,
                          const rosetta::core::Registry::MetadataHolder *h) {
        // Build class declaration
        auto base = h->get_base_class();
        std::string base_spec;
        if (!base.empty()) {
            size_t pos = base.rfind("::");
            base_spec = "(" + (pos != std::string::npos ? base.substr(pos + 2) : base) + ")";
        }

        line("class " + name + base_spec + ":");
        
        with_indent([&]() {
            line("\"\"\"" + name + " - C++ class bound via pybind11\"\"\"");

            // Constructor stubs
            auto ctors = h->get_constructors();
            if (ctors.empty()) {
                line("def __init__(self) -> None: ...");
            } else {
                for (const auto &ctor : ctors) {
                    line("def __init__(" + build_params("self", ctor.get_param_types()) + ") -> None: ...");
                }
            }

            // Method stubs
            for (const auto &m : h->get_methods()) {
                auto info = h->get_method_info(m);
                auto return_type = cpp_to_py(info.get_return_type_str());
                line("def " + m + "(" + build_params("self", info.get_param_types_str()) + ") -> " + return_type + ": ...");
            }
        });
        
        blank();
    }

    void write_free_function_stubs() {
        auto &func_registry = rosetta::core::FunctionRegistry::instance();
        auto function_names = func_registry.list_functions();
        
        if (function_names.empty()) return;
        
        line("# Free functions");
        for (const auto &func_name : function_names) {
            const auto &func_meta = func_registry.get(func_name);
            
            // Build parameter types as strings
            std::vector<std::string> param_type_strs;
            for (const auto &pt : func_meta.param_types()) {
                param_type_strs.push_back(rosetta::demangle(pt.name()));
            }
            
            auto return_type = cpp_to_py(rosetta::demangle(func_meta.return_type().name()));
            line("def " + func_name + "(" + build_params("", param_type_strs) + ") -> " + return_type + ": ...");
        }
        blank();
    }

    void write_utility_function_stubs() {
        emit(R"(
# Utility functions
def list_classes() -> List[str]: ...
def list_functions() -> List[str]: ...
def get_class_methods(class_name: str) -> List[str]: ...
def get_class_fields(class_name: str) -> List[str]: ...
def get_class_properties(class_name: str) -> List[str]: ...
def version() -> str: ...
)");
    }

    // Helper to build parameter list string
    std::string build_params(const std::string &first_param, 
                             const std::vector<std::string> &param_types) {
        std::string result = first_param;
        for (size_t i = 0; i < param_types.size(); ++i) {
            if (!result.empty()) result += ", ";
            result += "arg" + std::to_string(i) + ": " + cpp_to_py(param_types[i]);
        }
        return result;
    }

    std::string cpp_to_py(const std::string &t) {
        if (t == "void") return "None";
        if (t == "bool") return "bool";
        if (t == "int" || t == "long" || t == "size_t") return "int";
        if (t == "float" || t == "double") return "float";
        if (t.find("string") != std::string::npos) return "str";
        if (t.find("Vector3") != std::string::npos) return "Vector3";
        if (t.find("Matrix33") != std::string::npos) return "Matrix33";
        if (t.find("vector<double>") != std::string::npos) return "NDArray[np.float64]";
        if (t.find("vector<int>") != std::string::npos) return "NDArray[np.int32]";
        if (t.find("function<") != std::string::npos) return "Callable[..., Any]";

        std::string r = t;
        size_t pos = r.rfind("::");
        if (pos != std::string::npos) r = r.substr(pos + 2);
        r.erase(std::remove(r.begin(), r.end(), '*'), r.end());
        r.erase(std::remove(r.begin(), r.end(), '&'), r.end());
        return r.empty() ? "Any" : r;
    }
};