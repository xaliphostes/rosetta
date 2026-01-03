#pragma once
#include "Surface.h"

class Model {
    std::vector<Surface> surfaces;

public:
    Model() = default;
    void                        addSurface(const Surface &surface) { surfaces.push_back(surface); }
    const std::vector<Surface> &getSurfaces() const { return surfaces; }
    void                        setSurfaces(const std::vector<Surface> &s) { surfaces = s; }
};
