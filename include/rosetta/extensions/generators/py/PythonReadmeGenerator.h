#pragma once
#include "../common/CodeWriter.h"

// ============================================================================
// Python README Generator - Modernized
// ============================================================================
class PythonReadmeGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        emit(R"(
# ${MODULE} - Python Bindings

Python bindings generated from Rosetta introspection.

## Build with CMake

```bash
mkdir build && cd build
cmake ..
make
```

## Build with pip

```bash
pip install .
```

## Usage

- Make sure you have a venv activated for the python version used to compile the lib.
- Go to the build directory where the lib is created and create a `test.py` file with the following code:

```python
import numpy as np
import ${MODULE}  # The generated pybind11 module

# Create a model
model = ${MODULE}.Model()

# Create a surface with points and triangle indices
# Points: 3 vertices as flat array [x0,y0,z0, x1,y1,z1, x2,y2,z2]
# Triangles: indices [0, 1, 2]
points = np.array([0, 0, 0, 1, 0, 0, 0, 1, 0], dtype=np.float64)
triangles = np.array([0, 1, 2], dtype=np.int32)

surface = ${MODULE}.Surface(points, triangles)
model.addSurface(surface)

# Print points
for i, p in enumerate(surface.points):
    print(f"Point {i}: {p.x} {p.y} {p.z}")

# Print triangles
for i, t in enumerate(surface.triangles):
    print(f"Triangle {i}: {t.a} {t.b} {t.c}")

# Transform the surface using a callback function
def transform_point(p):
    return ${MODULE}.Point(p.x * 2 + 1, p.y * 2 + 1, p.z * 2 + 1)

surface.transform(transform_point)

# Print transformed points
for i, p in enumerate(surface.points):
    print(f"Point {i}: {p.x} {p.y} {p.z}")
```

- Then launch `python test.py`

**Expected output:**

```
Point 0: 0.0 0.0 0.0
Point 1: 1.0 0.0 0.0
Point 2: 0.0 1.0 0.0
Triangle 0: 0 1 2
Point 0: 1.0 1.0 1.0
Point 1: 3.0 1.0 1.0
Point 2: 1.0 3.0 1.0
```
)", {{"MODULE", config_.module_name}});
    }
};