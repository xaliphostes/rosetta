const Module = require('./build/Release/alljs.node')

const surface = new Module.Surface([0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1], [0, 1, 2])

// surface.points = [0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1]
// surface.triangles = [0, 1, 2]

console.log("nb points.  ", surface.points.length)
console.log("nb triangles", surface.triangles.length)

console.log("surface.points.forEach")
surface.points.forEach(p => console.log(`Point(${p.x}, ${p.y}, ${p.z})`))

console.log("surface.triangles.forEach")
surface.triangles.forEach(t => console.log(`Triangle(${t.a}, ${t.b}, ${t.c})`))

surface.transform(p => new Module.Point(p.x, p.y, 100 * p.z))

const model = new Module.Model()
model.addSurface(surface)

console.log("")
model.surfaces.forEach(s => {
    s.points.forEach(p => console.log(p.x, p.y, p.z))
    s.triangles.forEach(t => console.log(t.a, t.b, t.c))
})

class TSurface extends Module.Surface {
    constructor() {
        super()
        this.prop = []
    }
}

const s = new TSurface()
s.prop = [1, 2, 3]

print("")
print(s)