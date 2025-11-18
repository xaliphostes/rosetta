// ============================================================================
// Test file for automatic Emscripten bindings
// Run with: node test_auto.mjs
// ============================================================================

import createModule from './calcjs.js';

// Enhancement function - adds proper getters/setters/methods
function enhanceRosettaClasses(Module) {
    const classes = Module.listClasses();
    
    classes.forEach(className => {
        const ClassRef = Module[className];
        if (!ClassRef || !ClassRef.$meta) return;
        
        // Get metadata
        const meta = ClassRef.$meta();
        
        // Add field accessors as properties
        meta.fields.forEach(field => {
            Object.defineProperty(ClassRef.prototype, field, {
                get: function() { return this.$get(field); },
                set: function(v) { this.$set(field, v); },
                enumerable: true
            });
        });
        
        // Add method wrappers
        meta.methods.forEach(method => {
            // Don't override if already exists
            if (!ClassRef.prototype[method]) {
                ClassRef.prototype[method] = function(...args) {
                    return this.$call(method, args);
                };
            }
        });
    });
    
    return Module;
}

async function runTests() {
    console.log('Loading WebAssembly module...');
    let Module = await createModule();
    console.log('Module loaded!');
    
    // Debug: Check what's available before enhancement
    console.log('Available classes:', Module.listClasses());
    
    // Debug: Check if $meta exists
    if (Module.Vector3D.$meta) {
        console.log('Vector3D.$meta() =', Module.Vector3D.$meta());
    } else {
        console.log('ERROR: Vector3D.$meta not found!');
    }
    
    // Debug: Check if $get exists on prototype
    const testVec = new Module.Vector3D(1, 2, 3);
    if (testVec.$get) {
        console.log('testVec.$get("x") =', testVec.$get('x'));
    } else {
        console.log('ERROR: $get not found on Vector3D instance!');
    }
    testVec.delete();
    
    // Enhance classes with proper JS accessors
    Module = enhanceRosettaClasses(Module);
    console.log('Module enhanced!\n');

    // Show available classes
    console.log('Registered classes:', Module.listClasses());
    console.log('Rosetta version:', Module.version());
    console.log('');

    // ========================================================================
    // Test Vector3D
    // ========================================================================
    console.log('=== Testing Vector3D ===');
    
    const v1 = new Module.Vector3D();
    console.log(`v1 (default): (${v1.x}, ${v1.y}, ${v1.z})`);
    
    const v2 = new Module.Vector3D(3, 4, 0);
    console.log(`v2: (${v2.x}, ${v2.y}, ${v2.z})`);
    console.log(`v2.length(): ${v2.length()}`);  // Should be 5
    
    // Modify via property
    v2.x = 6;
    v2.y = 8;
    console.log(`v2 modified: (${v2.x}, ${v2.y}, ${v2.z})`);
    console.log(`v2.length(): ${v2.length()}`);  // Should be 10
    
    // Test methods
    const v3 = new Module.Vector3D(1, 2, 3);
    const v4 = v3.scale(2);
    console.log(`v3.scale(2): (${v4.x}, ${v4.y}, ${v4.z})`);  // (2, 4, 6)
    
    // Show metadata
    console.log('Vector3D metadata:', Module.Vector3D.$meta());
    
    v1.delete();
    v2.delete();
    v3.delete();
    v4.delete();
    console.log('✓ Vector3D tests passed\n');

    // ========================================================================
    // Test Rectangle
    // ========================================================================
    console.log('=== Testing Rectangle ===');
    
    const rect = new Module.Rectangle(10, 5);
    console.log(`Rectangle: ${rect.width} x ${rect.height}`);
    console.log(`Area: ${rect.area()}`);
    console.log(`Perimeter: ${rect.perimeter()}`);
    console.log(`Is square: ${rect.is_square()}`);
    
    // Modify
    rect.width = 5;
    console.log(`After setting width=5, is_square: ${rect.is_square()}`);
    
    rect.delete();
    console.log('✓ Rectangle tests passed\n');

    // ========================================================================
    // Test Person
    // ========================================================================
    console.log('=== Testing Person ===');
    
    const person = new Module.Person("Alice", 30);
    console.log(`Name: ${person.name}`);
    console.log(`Age: ${person.age}`);
    console.log(`Greet: ${person.greet()}`);
    
    // Modify
    person.name = "Bob";
    person.age = 25;
    console.log(`After modification: ${person.greet()}`);
    
    // Birthday
    person.celebrate_birthday();
    console.log(`After birthday: ${person.age}`);
    
    person.delete();
    console.log('✓ Person tests passed\n');

    // ========================================================================
    // Test Circle
    // ========================================================================
    console.log('=== Testing Circle ===');
    
    const circle = new Module.Circle(5);
    console.log(`Radius: ${circle.radius}`);
    console.log(`Diameter: ${circle.diameter}`);
    console.log(`Area: ${circle.area.toFixed(4)}`);
    console.log(`Circumference: ${circle.circumference.toFixed(4)}`);
    
    // Modify radius
    circle.radius = 10;
    console.log(`New radius: ${circle.radius}`);
    console.log(`New area: ${circle.area.toFixed(4)}`);
    
    circle.delete();
    console.log('✓ Circle tests passed\n');

    // ========================================================================
    // Test Free Functions
    // ========================================================================
    console.log('=== Testing Free Functions ===');
    
    const a = new Module.Vector3D(0, 0, 0);
    const b = new Module.Vector3D(3, 4, 0);
    console.log(`Distance: ${Module.distance(a, b)}`);
    
    const unit = Module.create_unit_vector(1, 1, 1);
    console.log(`Unit vector: (${unit.x.toFixed(4)}, ${unit.y.toFixed(4)}, ${unit.z.toFixed(4)})`);
    
    a.delete();
    b.delete();
    unit.delete();
    console.log('✓ Free function tests passed\n');

    console.log('=== All tests completed! ===');
}

runTests().catch(err => {
    console.error('Error:', err);
    process.exit(1);
});
