import os
import sys

sys.path.insert(
    0, os.path.join(os.path.dirname(__file__), "bindings", "python", "build")
)

import pygeom

s = pygeom.Surface([0,0,0, 1,0,0, 0,1,0], [0,1,2])

model = pygeom.Model()
model.addSurface(s)

for s in model.getSurfaces():
    print("Model surface points   :", [(n.x,n.y,n.z) for n in s.getPoints()])
    print("Model surface triangles:", [(t.a,t.b,t.c) for t in s.getTriangles()])
