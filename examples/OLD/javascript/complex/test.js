const addon = require('./build/Release/complex.node');

// TODO
// const surface = new addon.Surface([0, 0, 0, 1, 0, 0, 0, 1, 0], [0, 1, 2])
const surface = new addon.Surface()
surface.positions = [0, 0, 0, 1, 0, 0, 0, 1, 0]
// surface.indices = [0, 1, 2]

console.log(surface)
console.log(surface.positions)
console.log(surface.indices)
surface.forEachPoint(p => console.log(p))

const model = new addon.Model()
model.addSurface(surface)
// model.forEachSurface(surface => console.log(surface))
