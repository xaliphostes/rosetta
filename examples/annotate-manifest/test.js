// Verifies that the out-of-line annotations (widget.ann.json, wired by the
// manifest's "annotations" field) reach the compiled N-API module — even though
// widget.h itself carries no annotations.
//
//   build the module first:
//     ./generator out
//     cd out/node && npm install && npm run build && cd ../..   # npm run build = cmake-js
//   then:
//     node test.js

const path = require("path");
const assert = require("assert");

const widget = require(path.join(__dirname, "out", "node", "widget.node"));

const w = new widget.Widget();

// readonly -> assignment throws
assert.throws(() => { w.id = 5; }, /read-only/, "'id' should be read-only");

// range{0,100} -> out-of-range throws, in-range works
w.count = 50;
assert.strictEqual(w.count, 50);
assert.throws(() => { w.count = 999; }, /out of range/, "'count' should reject out-of-range");

// plain fields still read/write
w.title = "Hello";
w.mode = "fast";
assert.strictEqual(w.title, "Hello");
assert.strictEqual(w.mode, "fast");

console.log("test.js OK — readonly and range from widget.ann.json all applied");
