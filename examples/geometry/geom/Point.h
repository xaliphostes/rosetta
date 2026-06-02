#pragma once

struct Point {

    [[ = rosetta::doc{"The x-coordinate of the point"} ]]
    double x;
    [[ = rosetta::doc{"The y-coordinate of the point"} ]]
    double y;
    [[ = rosetta::doc{"The z-coordinate of the point"} ]]
    double z;
    Point() : x(0), y(0), z(0) {}
    Point(double x, double y, double z) : x(x), y(y), z(z) {}
};