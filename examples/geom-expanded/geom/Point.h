#pragma once

// Stock C++ — no annotations, no rosetta include. The per-field docs live out
// of line in Point.ann.json, wired in by the manifest's "annotations" field.
struct Point {
    double x;
    double y;
    double z;
    Point() : x(0), y(0), z(0) {}
    Point(double x, double y, double z) : x(x), y(y), z(z) {}
};
