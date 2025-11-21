import createModule from './allemjs.js'

async function runTests() {
    console.log('Loading WebAssembly module...');
    let Module = await createModule();
    console.log('Module loaded!');

    const surface = new Module.Surface([0.1, 0.1, 0.1, 1.1, 0.1, 0.1, 0.1, 1.1, 0.1], [0, 1, 2])

    surface.points.forEach(p => console.log(`Point(${p.x}, ${p.y}, ${p.z})`))
    surface.triangles.forEach(t => console.log(`Triangle(${t.a}, ${t.b}, ${t.c})`))

    surface.transform(p => new Module.Point(p.x, p.y, 100 * p.z))

    const model = new Module.Model()
    model.addSurface(surface)

    model.surfaces.forEach(s => {
        s.points.forEach(p => console.log(p.x, p.y, p.z))
        s.triangles.forEach(t => console.log(t.a, t.b, t.c))
    })

    class TSurface extends Module.Surface {
        constructor() {
            super()
            this.prop = []
        }
    }

    // Trying to instantiate a JavaScript class (TSurface) that extends
    // an Emscripten-bound C++ class (Module.Surface), but Emscripten
    // doesn't support this pattern directly
    /*
    const s = new TSurface()
    s.prop = [1, 2, 3]

    print("")
    print(s)
    */
}

runTests().catch(err => {
    console.error('Error:', err)
    process.exit(1)
})
