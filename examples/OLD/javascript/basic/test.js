const addon = require('./build/Release/geometry.node');

console.log('='.repeat(70));
console.log('Rosetta JavaScript Binding Test Suite');
console.log('='.repeat(70));

// Test 1: Basic Field Access
console.log('\n[Test 1] Basic Field Access');
console.log('-'.repeat(70));
const vec = new addon.Vector3D();
console.log('Initial values (should be 0):');
console.log('  x:', vec.x);
console.log('  y:', vec.y);
console.log('  z:', vec.z);

vec.x = 3;
vec.y = 4;
vec.z = 0;
console.log('\nAfter setting to (3, 4, 0):');
console.log('  x:', vec.x, '(expected: 3)');
console.log('  y:', vec.y, '(expected: 4)');
console.log('  z:', vec.z, '(expected: 0)');

// Test 2: Method Calls - Simple Return Type
console.log('\n[Test 2] Method Call with Simple Return Type');
console.log('-'.repeat(70));
const length = vec.length();
console.log('  vec.length():', length, '(expected: 5)');

// Test 3: Method Calls - Complex Return Type (Vector)
console.log('\n[Test 3] Method Call with Complex Return Type (std::vector)');
console.log('-'.repeat(70));
const arr = vec.to_array();
console.log('  vec.to_array():', arr);
console.log('  Type:', typeof arr);
console.log('  Is Array:', Array.isArray(arr));
if (Array.isArray(arr)) {
    console.log('  Elements: [' + arr.join(', ') + '] (expected: [3, 4, 0])');
}

// Test 4: Complex Field Types - String
console.log('\n[Test 4] String Field Access');
console.log('-'.repeat(70));
const container = new addon.DataContainer();
console.log('Initial name:', container.name, '(expected: empty string)');
container.name = "Test Container";
console.log('After setting:', container.name, '(expected: "Test Container")');

// Test 5: Complex Field Types - Vector
if (1) {
    console.log('\n[Test 5] Vector Field Access');
    console.log('-'.repeat(70));
    console.log('Initial values:', container.values, '(expected: empty array or undefined)');
    container.values = [1, 2, 3, 4, 5];
    console.log('After setting:', container.values, '(expected: [1, 2, 3, 4, 5])');
    if (Array.isArray(container.values)) {
        console.log('  Length:', container.values.length);
        console.log('  Sum:', container.values.reduce((a, b) => a + b, 0), '(expected: 15)');
    }
}

// Test 6: Complex Field Types - Optional
console.log('\n[Test 6] Optional Field Access');
console.log('-'.repeat(70));
console.log('Initial threshold:', container.threshold, '(expected: null or undefined)');
container.threshold = 42.5;
console.log('After setting:', container.threshold, '(expected: 42.5)');
console.log('  Type:', typeof container.threshold);

// Test 7: Type Introspection
console.log('\n[Test 7] Type Information System');
console.log('-'.repeat(70));

const intInfo = addon.inspectType('integer');
if (intInfo) {
    console.log('int type info:');
    console.log('  name:', intInfo.name);
    console.log('  size:', intInfo.size, 'bytes');
    console.log('  is numeric:', intInfo.isNumeric);
    console.log('  is integer:', intInfo.isInteger);
}

const vecInfo = addon.inspectType('Vector3D');
if (vecInfo) {
    console.log('\nVector3D type info:');
    console.log('  name:', vecInfo.name);
    console.log('  size:', vecInfo.size, 'bytes');
    console.log('  alignment:', vecInfo.alignment, 'bytes');
    console.log('  is template:', vecInfo.isTemplate);
}

// Test 8: List All Classes
console.log('\n[Test 8] Available Classes');
console.log('-'.repeat(70));
const classes = addon.listClasses();
console.log('Registered classes:', classes);
console.log('Count:', classes.length);

// Test 9: Multiple Instances
console.log('\n[Test 9] Multiple Instances');
console.log('-'.repeat(70));
const vec1 = new addon.Vector3D();
const vec2 = new addon.Vector3D();

vec1.x = 1;
vec1.y = 0;
vec1.z = 0;

vec2.x = 0;
vec2.y = 1;
vec2.z = 0;

console.log('vec1: (', vec1.x, ',', vec1.y, ',', vec1.z, ') length:', vec1.length());
console.log('vec2: (', vec2.x, ',', vec2.y, ',', vec2.z, ') length:', vec2.length());
console.log('Instances are independent:', vec1.x !== vec2.x);

// Test 10: Edge Cases
console.log('\n[Test 10] Edge Cases');
console.log('-'.repeat(70));

// Zero vector
const zeroVec = new addon.Vector3D();
console.log('Zero vector length:', zeroVec.length(), '(expected: 0)');

// Large numbers
vec.x = 1e6;
vec.y = 2e6;
vec.z = 3e6;
console.log('Large values: (', vec.x, ',', vec.y, ',', vec.z, ')');
console.log('  Length:', vec.length().toExponential(2));

// Negative numbers
vec.x = -3;
vec.y = -4;
vec.z = 0;
console.log('Negative values: (', vec.x, ',', vec.y, ',', vec.z, ')');
console.log('  Length:', vec.length(), '(expected: 5)');

// Summary
console.log('\n' + '='.repeat(70));
console.log('Test Suite Complete');
console.log('='.repeat(70));
console.log('\nIf all values match expectations, the binding system is working correctly!');
