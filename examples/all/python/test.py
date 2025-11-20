#!/usr/bin/env python3

import allpy as rosetta

surface = rosetta.Surface(
    [0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1],
    [0, 1, 2]
)

print("")
for p in surface.points:
    print(f"Point({p.x}, {p.y}, {p.z})")

print("")
for t in surface.triangles:
    print(f"Triangle({t.a}, {t.b}, {t.c})")

surface.transform(lambda p: rosetta.Point(p.x, p.y, 100*p.z))

model = rosetta.Model()
model.addSurface(surface)

print("")
for s in model.surfaces:
    for p in s.points:
        print(p.x, p.y, p.z)
    for t in s.triangles:
        print(t.a, t.b, t.c)

# Python class inheritance
class TSurface(rosetta.Surface):
    def __init__(self):
        super().__init__()
        self.prop = []

s = TSurface()
s.prop = [1, 2, 3]

print("")
print(s)