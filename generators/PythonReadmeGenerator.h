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
        line("- Make sure you have a venv activated for the python version used to compile the "
             "lib.");
        line("- Go to the build directory where the lib is created and create a `test.py` file "
             "with the following code:");
        line();
        line("```python");
        line("import numpy as np");
        line("import " + config_.module_name + "  # The generated pybind11 module");
        line();
        line("# Create a model");
        line("model = " + config_.module_name + ".Model()");
        line();
        line("# Create a surface with points and triangle indices");
        line("# Points: 3 vertices as flat array [x0,y0,z0, x1,y1,z1, x2,y2,z2]");
        line("# Triangles: indices [0, 1, 2]");
        line("points = np.array([0, 0, 0, 1, 0, 0, 0, 1, 0], dtype=np.float64)");
        line("triangles = np.array([0, 1, 2], dtype=np.int32)");
        line();
        line("surface = " + config_.module_name + ".Surface(points, triangles)");
        line("model.addSurface(surface)");
        line();
        line("# Print points");
        line("for i, p in enumerate(surface.points):");
        line("    print(f\"Point {i}: {p.x} {p.y} {p.z}\")");
        line();
        line("# Print triangles");
        line("for i, t in enumerate(surface.triangles):");
        line("    print(f\"Triangle {i}: {t.a} {t.b} {t.c}\")");
        line();
        line("# Transform the surface using a callback function");
        line("def transform_point(p):");
        line("    return " + config_.module_name + ".Point(p.x * 2 + 1, p.y * 2 + 1, p.z * 2 + 1)");
        line();
        line("surface.transform(transform_point)");
        line();
        line("# Print transformed points");
        line("for i, p in enumerate(surface.points):");
        line("    print(f\"Point {i}: {p.x} {p.y} {p.z}\")");
        line("```");
        line();
        line("- Then launch `python test.py`");
        line();
        line("**Expected output:**");
        line();
        line("```");
        line("Point 0: 0.0 0.0 0.0");
        line("Point 1: 1.0 0.0 0.0");
        line("Point 2: 0.0 1.0 0.0");
        line("Triangle 0: 0 1 2");
        line("Point 0: 1.0 1.0 1.0");
        line("Point 1: 3.0 1.0 1.0");
        line("Point 2: 1.0 3.0 1.0");
        line("```");
    }
};