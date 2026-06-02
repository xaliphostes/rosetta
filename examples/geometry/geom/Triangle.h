#pragma once
#include "Kind.h"

struct Triangle {
    int a, b, c;

    [[ = rosetta::doc{"What kind of primitive this triangle represents"} ]]
    Kind kind = Kind::Surface;

    Triangle() : a(0), b(0), c(0) {}
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};
