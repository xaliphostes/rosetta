const rosetta = require('./build/Release/rosetta')

const s = new rosetta.Surface([0., 1, 2, 3, 4, 5, 6, 7, 8], [0, 1, 2])
console.log('s.vertices():', s.vertices())
console.log('s.triangles():', s.triangles())

s.setVertices([8, 7, 6, 5, 4, 3, 2, 1, 0])
console.log('s.vertices():', s.vertices())
