#include "Vector3.h"
#include <cmath>

namespace space {

    Vector3::Vector3() : x(0.0), y(0.0), z(0.0) {
    }

    Vector3::Vector3(double x, double y, double z) : x(x), y(y), z(z) {
    }

    double Vector3::length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    double Vector3::dot(const Vector3 &o) const {
        return x * o.x + y * o.y + z * o.z;
    }

    Vector3 Vector3::cross(const Vector3 &o) const {
        return Vector3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    Vector3 Vector3::normalized() const {
        const double len = length();
        if (len == 0.0) {
            return Vector3(0.0, 0.0, 0.0);
        }
        return Vector3(x / len, y / len, z / len);
    }

    Vector3 Vector3::add(const Vector3 &o) const {
        return Vector3(x + o.x, y + o.y, z + o.z);
    }

    Vector3 Vector3::scale(double f) const {
        return Vector3(x * f, y * f, z * f);
    }

} // namespace space
