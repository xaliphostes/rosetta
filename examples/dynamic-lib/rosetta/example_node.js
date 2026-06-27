// node (reflection) binding for the `space` shared library.
//
//   cd bindings/node && npm install && npm run build && cd -
//   node example_node.js
//
// The addon's rpath points at ../space/bin, so libspace.dylib (where every
// method body actually lives) is found at load time wherever you run from.

const path = require("path");
const space = require(path.join(__dirname, "bindings", "node", "space.node"));

const v = new space.Vector3(3, 0, 4);
console.log("Vector3(3, 0, 4).length() =", v.length()); // 5 — from libspace
const n = v.normalized();
console.log("           .normalized() =", [n.x, n.y, n.z]); // [0.6, 0, 0.8]

const a = new space.Vector3(1, 2, 3);
const b = new space.Vector3(4, 5, 6);
console.log("a.dot(b)   =", a.dot(b)); // 32
const c = a.cross(b);
console.log("a.cross(b) =", [c.x, c.y, c.z]); // [-3, 6, -3]

const box = new space.BoundingBox(new space.Vector3(0, 0, 0), new space.Vector3(2, 2, 2));
const ctr = box.center();
console.log("box.center()   =", [ctr.x, ctr.y, ctr.z]); // [1, 1, 1]
console.log("box.diagonal() =", box.diagonal().toFixed(4)); // 3.4641
console.log("box.contains((1,1,1)) =", box.contains(new space.Vector3(1, 1, 1)));
