#pragma once
#include "../common/CodeWriter.h"

// ============================================================================
// Python pyproject.toml Generator - Modernized
// ============================================================================
class PythonPyprojectGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        emit(R"(
[build-system]
requires = ["setuptools>=45", "wheel", "pybind11>=2.10"]
build-backend = "setuptools.build_meta"

[project]
name = "${MODULE}"
version = "${VERSION}"
description = "${DESCRIPTION}"
readme = "README.md"
license = {text = "${LICENSE}"}
requires-python = ">=3.8"
dependencies = ["numpy"]

[project.optional-dependencies]
dev = ["pytest", "mypy"]
)",
             {{"MODULE", config_.module_name},
              {"VERSION", config_.version},
              {"DESCRIPTION", config_.description},
              {"LICENSE", config_.license}});
    }
};