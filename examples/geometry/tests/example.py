import geom

model = geom.Model()

surface = geom.Surface([0,0,0, 1,0,0, 0,1,0], [0,1,2])
model.addSurface(surface)

print("vertices:")
for p in surface.points:
    print(" ", p.x, p.y, p.z)

print("triangles:")
for p in surface.triangles:
    print(" ", p.a, p.b, p.c)
