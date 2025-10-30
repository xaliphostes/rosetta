const addon = require('./build/Release/geometry.node');

console.log('Rosetta Geometry Module Loaded');
console.log('======================================================================');

// Create objects
const vec = new addon.Vector3D();
vec.x = 3;
vec.y = 4;
vec.z = 0;

console.log('Length:', vec.length());
console.log('x:', vec.x);
console.log('y:', vec.y);
console.log('z:', vec.z);

// Vector with arrays
const arr = vec.to_array();
console.log('Array:', arr);

// Data container with complex types
const container = new addon.DataContainer();
container.name = "Test";

container.values = [1, 2, 3, 4, 5];
container.threshold = 42.5;

console.log('Container:', container.name);
console.log('About to access container.values...');
try {
    const values = container.values;
    console.log('Successfully got values:', values);
    console.log('Values type:', typeof values);
    console.log('Values is array:', Array.isArray(values));
    if (Array.isArray(values)) {
        console.log('Values length:', values.length);
    }
} catch(e) {
    console.log('Error accessing values:', e);
}
console.log('Threshold:', container.threshold);

// Inspect type information
const intInfo = addon.inspectType('integer');
console.log('int type info:', intInfo);

const vecInfo = addon.inspectType('Vector3D');
console.log('Vector3D type info:', vecInfo);

// List all types
console.log('Available types:', addon.listClasses());