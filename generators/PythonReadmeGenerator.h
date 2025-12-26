#pragma once
#include "CodeWriter.h"

// ============================================================================
// Python README Generator
// ============================================================================
class PythonReadmeGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("# " + config_.module_name + " - Python Bindings");
        line();
        line("Python bindings generated from Rosetta introspection.");
        line();
        line("## Build with CMake");
        line();
        line("```bash");
        line("mkdir build && cd build");
        line("cmake ..");
        line("make");
        line("```");
        line();
        line("## Build with pip");
        line();
        line("```bash");
        line("pip install .");
        line("```");
        line();
        line("## Usage");
        line();
        line("```python");
        line("import " + config_.module_name);
        line();
        line("# List available classes");
        line("print(" + config_.module_name + ".list_classes())");
        line();
        line("# Create objects");
        line("model = " + config_.module_name + ".Model()");
        line("```");
    }
};