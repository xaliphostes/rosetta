#pragma once

#include "Model.h"
#include "Point.h"
#include "Surface.h"
#include "Triangle.h"

Point transform(const Point &p) {
    return Point(p.x * 2, p.z * 3, p.y * 4);
}
