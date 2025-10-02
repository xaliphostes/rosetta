const rosetta = require('./build/Release/js2')

const a = new rosetta.A()
console.log('a:', a)

a.x = 42
console.log('a.x:', a.x)
console.log('a.getX():', a.getX())

a.setX(123)
console.log('a.x:', a.x)
console.log('a.getX():', a.getX())

const b = new rosetta.B()

console.log('b.a:', b.a())
console.log('b.a.x:', b.a().x)

b.setX(7)
console.log('b.a.x:', b.a().x)