const rosetta = require('./build/Release/rosetta')

const a = new rosetta.A()
console.log(a.stress())

a.setStress([1, 2, 3, 4, 5, 6])
console.log(a.stress())
