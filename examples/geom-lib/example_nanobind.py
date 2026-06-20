# Same demo as example.py, but against the nanobind backend (target "nanobind",
# module "nbgeom") instead of pybind11 (target "python", module "pygeom").
#
# rosetta gives both backends an identical Python API, so the only differences
# from example.py are the two lines below: the build directory and the module
# name. Build first:
#   ( cd bindings/nanobind && cmake -S . -B build && cmake --build build -j )
import os
import sys

sys.path.insert(
    0, os.path.join(os.path.dirname(__file__), "bindings", "nanobind", "build")
)

import nbgeom as geom  # the only real change vs example.py (pygeom -> nbgeom)

s = geom.Surface([0, 0, 0, 1, 0, 0, 0, 1, 0], [0, 1, 2])

model = geom.Model()
model.addSurface(s)

for s in model.getSurfaces():
    print("Model surface points   :", [(n.x, n.y, n.z) for n in s.getPoints()])
    print("Model surface triangles:", [(t.a, t.b, t.c) for t in s.getTriangles()])

# `transform` is a free (non-member) function bound from common.h. It takes a
# Point and returns a new Point swizzled to (x*2, z*3, y*4).
p = geom.Point(1, 2, 3)
q = geom.transform(p)
print("transform(1, 2, 3) =>", (q.x, q.y, q.z))
