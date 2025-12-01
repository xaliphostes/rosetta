// ============================================================================
// Example: Using the non-intrusive Python binding generator with a DYNAMIC library
// ============================================================================

#include <cmath>
#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>
#include "../flower/include/flower.h"

void register_rosetta_classes() {
    ROSETTA_REGISTER_CLASS(Flower)
        .method("computeCircleArea", &Flower::computeCircleArea)
        .method("computeSphereVolume", &Flower::computeSphereVolume)
        .method("computeFibonacci", &Flower::computeFibonacci);
}

// ============================================================================
// Python Module Definition
// ============================================================================

BEGIN_PY_MODULE(pyflower, "Python bindings for C++ classes using Rosetta introspection (dynamic lib)") {
    register_rosetta_classes();
    BIND_PY_CLASS(Flower);
}
END_PY_MODULE()