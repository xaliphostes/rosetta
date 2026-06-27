#include "BoundingBox.h"

namespace space {

    BoundingBox::BoundingBox() : min(), max() {
    }

    BoundingBox::BoundingBox(const Vector3 &min, const Vector3 &max) : min(min), max(max) {
    }

    Vector3 BoundingBox::center() const {
        return Vector3((min.x + max.x) * 0.5, (min.y + max.y) * 0.5, (min.z + max.z) * 0.5);
    }

    Vector3 BoundingBox::size() const {
        return Vector3(max.x - min.x, max.y - min.y, max.z - min.z);
    }

    double BoundingBox::diagonal() const {
        return size().length();
    }

    bool BoundingBox::contains(const Vector3 &p) const {
        return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y && p.z >= min.z &&
               p.z <= max.z;
    }

} // namespace space
