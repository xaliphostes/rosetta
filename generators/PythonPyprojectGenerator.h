#pragma once
#include "CodeWriter.h"

// ============================================================================
// Python pyproject.toml Generator
// ============================================================================
class PythonPyprojectGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("[build-system]");
        line("requires = [\"setuptools>=45\", \"wheel\", \"pybind11>=2.10\"]");
        line("build-backend = \"setuptools.build_meta\"");
        line();
        line("[project]");
        line("name = \"" + config_.module_name + "\"");
        line("version = \"" + config_.version + "\"");
        line("description = \"" + config_.description + "\"");
        line("readme = \"README.md\"");
        line("license = {text = \"" + config_.license + "\"}");
        line("requires-python = \">=3.8\"");
        line("dependencies = [\"numpy\"]");
        line();
        line("[project.optional-dependencies]");
        line("dev = [\"pytest\", \"mypy\"]");
    }
};
