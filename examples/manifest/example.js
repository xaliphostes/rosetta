const path = require("path");

const manifest = require(path.join(__dirname, "bindings", "node", "manifest.node"))


const p = new manifest.Person("Toto", 18, "xd678shg")
console.log(p.name)
console.log(p.age)
console.log(p.id)

console.log(p.greet("Hello"))
