#pragma once
#include "Kind.h"

// Stock C++ — no annotations, no rosetta include. The per-field docs and the
// range validation on a/b/c live out of line in Triangle.ann.json.
struct Triangle {
    int  a;
    int  b;
    int  c;
    Kind kind = Kind::Surface;

    Triangle() : a(0), b(0), c(0) {}
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};
