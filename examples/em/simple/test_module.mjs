// ============================================================================
// Test file for the Emscripten-compiled module
// Run with: node test_module.js
// ============================================================================

// Import the module (adjust path as needed)
import createModule from './calcjs.js';

async function runTests() {
    console.log('Loading WebAssembly module...');
    const Module = await createModule();
    console.log('Module loaded!\n');

    // ========================================================================
    // Test Vector3D
    // ========================================================================
    console.log('=== Testing Vector3D ===');
    
    // Default constructor
    const v1 = new Module.Vector3D();
    console.log(`v1 (default): (${v1.x}, ${v1.y}, ${v1.z})`);
    
    // Parameterized constructor
    const v2 = new Module.Vector3D(3, 4, 0);
    console.log(`v2: (${v2.x}, ${v2.y}, ${v2.z})`);
    console.log(`v2.length(): ${v2.length()}`);  // Should be 5
    
    // Normalize
    const v3 = new Module.Vector3D(10, 0, 0);
    v3.normalize();
    console.log(`v3 normalized: (${v3.x}, ${v3.y}, ${v3.z})`);  // Should be (1, 0, 0)
    
    // Add
    const v4 = new Module.Vector3D(1, 2, 3);
    const v5 = new Module.Vector3D(4, 5, 6);
    const v6 = v4.add(v5);
    console.log(`v4 + v5: (${v6.x}, ${v6.y}, ${v6.z})`);  // Should be (5, 7, 9)
    
    // Scale
    const v7 = v4.scale(2);
    console.log(`v4 * 2: (${v7.x}, ${v7.y}, ${v7.z})`);  // Should be (2, 4, 6)
    
    // Clean up
    v1.delete();
    v2.delete();
    v3.delete();
    v4.delete();
    v5.delete();
    v6.delete();
    v7.delete();
    
    console.log('');

    // ========================================================================
    // Test Rectangle
    // ========================================================================
    console.log('=== Testing Rectangle ===');
    
    const rect = new Module.Rectangle(10, 5);
    console.log(`Rectangle: ${rect.width} x ${rect.height}`);
    console.log(`Area: ${rect.area()}`);  // Should be 50
    console.log(`Perimeter: ${rect.perimeter()}`);  // Should be 30
    console.log(`Is square: ${rect.is_square()}`);  // Should be false
    
    const square = new Module.Rectangle(5, 5);
    console.log(`Square is_square: ${square.is_square()}`);  // Should be true
    
    rect.delete();
    square.delete();
    
    console.log('');

    // ========================================================================
    // Test Person
    // ========================================================================
    console.log('=== Testing Person ===');
    
    const person = new Module.Person("Alice", 30);
    console.log(`Name: ${person.name}`);
    console.log(`Age: ${person.age}`);
    console.log(`Greeting: ${person.greet()}`);
    
    // Modify properties
    person.name = "Bob";
    person.age = 25;
    console.log(`After modification: ${person.greet()}`);
    
    // Birthday
    person.celebrate_birthday();
    console.log(`After birthday: Age = ${person.age}`);  // Should be 26
    
    person.delete();
    
    console.log('');

    // ========================================================================
    // Test Circle
    // ========================================================================
    console.log('=== Testing Circle ===');
    
    const circle = new Module.Circle(5);
    console.log(`Radius: ${circle.radius}`);
    console.log(`Diameter: ${circle.diameter}`);  // Should be 10
    console.log(`Area: ${circle.area.toFixed(4)}`);  // Should be ~78.5398
    console.log(`Circumference: ${circle.circumference.toFixed(4)}`);  // Should be ~31.4159
    
    // Modify radius
    circle.radius = 10;
    console.log(`New radius: ${circle.radius}`);
    console.log(`New area: ${circle.area.toFixed(4)}`);  // Should be ~314.1593
    
    circle.delete();
    
    console.log('');

    // ========================================================================
    // Test Free Functions
    // ========================================================================
    console.log('=== Testing Free Functions ===');
    
    const a = new Module.Vector3D(0, 0, 0);
    const b = new Module.Vector3D(3, 4, 0);
    const dist = Module.distance(a, b);
    console.log(`Distance from origin to (3,4,0): ${dist}`);  // Should be 5
    
    const unit = Module.create_unit_vector(1, 1, 1);
    console.log(`Unit vector: (${unit.x.toFixed(4)}, ${unit.y.toFixed(4)}, ${unit.z.toFixed(4)})`);
    
    a.delete();
    b.delete();
    unit.delete();
    
    console.log('');

    // ========================================================================
    // Test Constants
    // ========================================================================
    console.log('=== Testing Constants ===');
    console.log(`PI: ${Module.PI}`);
    
    console.log('\n=== All tests completed! ===');
}

// Run tests
runTests().catch(err => {
    console.error('Error:', err);
    process.exit(1);
});
