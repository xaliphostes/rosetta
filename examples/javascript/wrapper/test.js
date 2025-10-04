const rosetta = require('./build/Release/rosetta')

const algo = new rosetta.Algo()
const res = algo.run(1e-6)
console.log('Result:', res)