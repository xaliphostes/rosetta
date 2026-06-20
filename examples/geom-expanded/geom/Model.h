#pragma once
#include "Surface.h"

// Stock C++ — no annotations, no rosetta include. The doc on addSurface lives
// out of line in Model.ann.json (side-car annotations target methods too).
class Model {
    std::vector<Surface> surfaces;

public:
    Model() = default;

    void                        addSurface(const Surface &surface) { surfaces.push_back(surface); }
    const std::vector<Surface> &getSurfaces() const { return surfaces; }
    void                        setSurfaces(const std::vector<Surface> &s) { surfaces = s; }
};
