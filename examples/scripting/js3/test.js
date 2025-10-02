const rosetta = require('./build/Release/js3')

let s = new rosetta.Surface()
console.log('s:', s)

s = new rosetta.Surface([0., 1, 2, 3, 4, 5, 6, 7, 8], [0, 1, 2])
console.log('s:', s)
console.log('s.vertices():', s.vertices())
console.log('s.triangles():', s.triangles())