// ============================================================================
// JavaScript test file demonstrating usage of Rosetta N-API bindings
// Run with: node test.js
// ============================================================================

const rosetta = require('./build/Release/rosetta');

console.log('='.repeat(60));
console.log('Rosetta N-API Bindings Test');
// console.log('Version:', rosetta.version());
console.log('='.repeat(60));

// ============================================================================
// Test 1: Basic Vector3D Usage
// ============================================================================

console.log('\n--- Test 1: Vector3D ---');
const vec = new rosetta.Vector3D(3, 4, 0);
console.log('Created vector:', { x: vec.x, y: vec.y, z: vec.z });

// Access fields
console.log('Length:', vec.length());

// Modify fields
vec.x = 10;
console.log('After setting x=10:', { x: vec.x, y: vec.y, z: vec.z });
console.log('New length:', vec.length());

// Call methods
vec.normalize();
console.log('After normalize:', { x: vec.x, y: vec.y, z: vec.z });
console.log('Normalized length:', vec.length());

// console.log('toString:', vec.to_string());

// ============================================================================
// Test 2: Inheritance with Sphere
// ============================================================================

console.log('\n--- Test 2: Sphere (Inheritance) ---');
const sphere = new rosetta.Sphere(5.0);
console.log('Created sphere with radius:', sphere.radius);

// Access base class fields
sphere.name = "Big Sphere";
sphere.position = { x: 1, y: 2, z: 3 };
console.log('Sphere name:', sphere.name);
console.log('Sphere position:', sphere.position);

// Call virtual methods
console.log('Volume:', sphere.volume());
console.log('Type:', sphere.get_type());

// ============================================================================
// Test 3: Box
// ============================================================================

console.log('\n--- Test 3: Box ---');
const box = new rosetta.Box(2, 3, 4);
console.log('Created box:', { width: box.width, height: box.height, depth: box.depth });
console.log('Volume:', box.volume());
console.log('Type:', box.get_type());

box.name = "My Box";
console.log('Box name:', box.name);

// ============================================================================
// Test 4: Containers (Arrays)
// ============================================================================

console.log('\n--- Test 4: Containers ---');
const a = new rosetta.A();

// Vector<double>
a.areas = [1.0, 2.0, 3.0, 4.0, 5.0];
console.log('Set areas:', a.areas);

// Vector<Vector3D>
a.positions = [
    { x: 1, y: 2, z: 3 },
    { x: 4, y: 5, z: 6 },
    { x: 7, y: 8, z: 9 }
];
console.log('Set positions (count):', a.positions.length);

// Map<string, uint32_t>
a.map = {
    "key1": 100,
    "key2": 200,
    "key3": 300
};
console.log('Set map:', a.map);

// Array<double, 9> - fixed size array
a.stress = [1, 2, 3, 4, 5, 6, 7, 8, 9];
console.log('Set stress tensor:', a.stress);

// Vector<Array<double, 9>>
a.stresses = [
    [1, 2, 3, 4, 5, 6, 7, 8, 9],
    [9, 8, 7, 6, 5, 4, 3, 2, 1]
];
console.log('Set stresses (count):', a.stresses.length);

// ============================================================================
// Test 5: Functors (Lambdas)
// ============================================================================

console.log('\n--- Test 5: Functors (JavaScript -> C++) ---');

// Function<double(double, double)>
a.calculator = (x, y) => {
    console.log(`  [JS] calculator called with x=${x}, y=${y}`);
    return x + y;
};

// Function<void()>
a.callback = () => {
    console.log('  [JS] callback invoked!');
};

// Function<Vector3D(const Vector3D&)>
a.transformer = (v) => {
    console.log(`  [JS] transformer called with (${v.x}, ${v.y}, ${v.z})`);
    return {
        x: v.x * 2,
        y: v.y * 2,
        z: v.z * 2
    };
};

// Now call C++ methods that will invoke these JS functors
console.log('Testing functors...');
// In a real scenario, you'd have C++ methods that call these functors
// For now, we demonstrate that they're set correctly
console.log('Calculator set:', typeof a.calculator);
console.log('Callback set:', typeof a.callback);
console.log('Transformer set:', typeof a.transformer);

// ============================================================================
// Test 6: Method Calls with Arrays
// ============================================================================

console.log('\n--- Test 6: Method Calls ---');

// Call setAreas method
a.setAreas([10, 20, 30, 40, 50]);
console.log('After setAreas:', a.areas);

// Call setPositions method
a.setPositions([
    { x: 0, y: 0, z: 0 },
    { x: 1, y: 1, z: 1 }
]);
console.log('After setPositions (count):', a.positions.length);

// ============================================================================
// Test 7: Error Handling
// ============================================================================

console.log('\n--- Test 7: Error Handling ---');

try {
    // Try to create a Shape (abstract class)
    const shape = new rosetta.Shape();
    console.log('ERROR: Should not be able to create abstract Shape!');
} catch (e) {
    console.log('Correctly caught error:', e.message);
}

try {
    // Try to set wrong type
    vec.x = "not a number";
    console.log('x is now:', vec.x);
} catch (e) {
    console.log('Correctly caught type error:', e.message);
}

// ============================================================================
// Test 8: Performance Check
// ============================================================================

console.log('\n--- Test 8: Performance ---');

const start = Date.now();
const iterations = 100000;

for (let i = 0; i < iterations; i++) {
    const v = new rosetta.Vector3D(i, i + 1, i + 2);
    const len = v.length();
}

const end = Date.now();
console.log(`Created ${iterations} Vector3D objects in ${end - start}ms`);
console.log(`Average: ${((end - start) / iterations * 1000).toFixed(2)} microseconds per object`);

// ============================================================================
// Summary
// ============================================================================

console.log('\n' + '='.repeat(60));
console.log('All tests completed successfully!');
console.log('='.repeat(60));

// ============================================================================
// Advanced Example: Combining Everything
// ============================================================================

console.log('\n--- Advanced Example ---');

function createScene() {
    const scene = new rosetta.A();

    // Set up geometry
    scene.positions = [
        { x: 0, y: 0, z: 0 },
        { x: 1, y: 0, z: 0 },
        { x: 0, y: 1, z: 0 },
        { x: 0, y: 0, z: 1 }
    ];

    // Calculate areas
    const areas = scene.positions.map(p => {
        const v = new rosetta.Vector3D(p.x, p.y, p.z);
        return v.length();
    });
    scene.areas = areas;

    // Set up a transformer
    scene.transformer = (v) => ({
        x: v.x + 10,
        y: v.y + 10,
        z: v.z + 10
    });

    // Add some metadata
    scene.map = {
        "vertices": 4,
        "dimension": 3,
        "version": 1
    };

    console.log('Scene created with:');
    console.log('  Positions:', scene.positions.length);
    console.log('  Areas:', scene.areas);
    console.log('  Metadata:', scene.map);

    return scene;
}

const myScene = createScene();
console.log('Scene setup complete!');