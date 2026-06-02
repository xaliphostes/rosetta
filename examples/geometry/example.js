const path = require("path");

// Load the compiled N-API addon. The node backend's CMakeLists copies the
// built `jsgeom.node` next to its source dir via a POST_BUILD step.
const jsgeom = require(
    path.join(__dirname, "bindings", "node", "jsgeom.node")
);

const model = new jsgeom.Model();

const surface = new jsgeom.Surface([0, 0, 0, 1, 0, 0, 0, 1, 0], [0, 1, 2]);
model.addSurface(surface);

for (const s of model.getSurfaces()) {
    console.log("Model surface points")
    for (const p of surface.getPoints()) {
        console.log(" ", p.x, p.y, p.z);
    }
    console.log("Model surface triangles")
    for (const t of surface.getTriangles()) {
        console.log(" ", t.a, t.b, t.c);
    }
}
