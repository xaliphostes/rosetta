const rosetta = require('./build/Release/rosetta')

const proc = new rosetta.DataProcessor([1, 2, 3, 4, 5])
console.log('Initial:', proc.data(), '\n')

// Test C++ → JavaScript functor
const mult = proc.multiplier(10)
const times3 = mult(3); // should be 30
console.log('times3:', times3, '\n')

// Test JavaScript → C++ functor
const filtered = proc.filter(x => x > 3)
console.log('After filter (> 3):', filtered, '\n')

console.log('forEach:')
proc.forEach(x => console.log('Element:', x))