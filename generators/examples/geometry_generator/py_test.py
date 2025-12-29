import numpy as np
import geometry  # The generated pybind11 module

# Create a model
model = geometry.Model()

# Create a surface with points and triangle indices
# Points: 3 vertices as flat array [x0,y0,z0, x1,y1,z1, x2,y2,z2]
# Triangles: indices [0, 1, 2]
points = np.array([0, 0, 0, 1, 0, 0, 0, 1, 0], dtype=np.float64)
triangles = np.array([0, 1, 2], dtype=np.int32)

surface = geometry.Surface(points, triangles)
model.addSurface(surface)

# Print points
for i, p in enumerate(surface.points):
    print(f"Point {i}: {p.x} {p.y} {p.z}")

# Print triangles
for i, t in enumerate(surface.triangles):
    print(f"Triangle {i}: {t.a} {t.b} {t.c}")

# Transform the surface using a callback function
def transform_point(p):
    return geometry.Point(p.x * 2 + 1, p.y * 2 + 1, p.z * 2 + 1)

surface.transform(transform_point)

# Print transformed points
for i, p in enumerate(surface.points):
    print(f"Point {i}: {p.x} {p.y} {p.z}")
