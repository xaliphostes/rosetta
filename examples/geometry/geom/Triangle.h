#pragma once
#include "Kind.h"

struct Triangle {
    [[ = rosetta::doc{"First vertex index"}, = rosetta::range{0, 1000000} ]]
    int a;
    [[ = rosetta::doc{"Second vertex index"}, = rosetta::range{0, 1000000} ]]
    int b;
    [[ = rosetta::doc{"Third vertex index"}, = rosetta::range{0, 1000000} ]]
    int c;

    [[ = rosetta::doc{"What kind of primitive this triangle represents"} ]]
    Kind kind = Kind::Surface;

    Triangle() : a(0), b(0), c(0) {}
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};
