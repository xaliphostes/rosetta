// load_geometry.mjs
import createGeometry from './geometry.js';

async function main() {
    // Initialize the module
    const Module = await createGeometry();
    
    // List available classes
    // console.log('Available classes:', Module.listClasses());
    
    // Create objects
    const point = new Module.Point(1.0, 2.0, 3.0);
    console.log('Point:', point.x, point.y, point.z);
    
    const triangle = new Module.Triangle(0, 1, 2);
    console.log('Triangle:', triangle.a, triangle.b, triangle.c);
    
    // Create a surface with points and indices
    const positions = new Float64Array([0, 0, 0, 1, 0, 0, 0, 1, 0]);
    const indices = new Int32Array([0, 1, 2]);
    const surface = Module.Surface.create(positions, indices);
    
    // Get points back
    const points = surface.points;
    console.log('Surface has', points.length, 'points');
    
    // Transform with a callback function
    surface.transform((p) => {
        return new Module.Point(p.x * 2, p.y * 2, p.z * 2);
    });
    
    // Clean up (prevent memory leaks)
    point.delete();
    triangle.delete();
    surface.delete();
}

main().catch(console.error);