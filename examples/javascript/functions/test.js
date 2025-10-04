const rosetta = require('./build/Release/rosetta');

console.log(rosetta.add(20, 3));  // 23
console.log(rosetta.greet("World"));  // "Hello, World"
console.log(rosetta.calculateDistance(0, 0, 3, 4));  // 5.0