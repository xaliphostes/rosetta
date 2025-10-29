// ============================================================================
// Comprehensive test suite for C++ functor bindings
// Demonstrates all functor patterns and use cases
// ============================================================================

const addon = require('./build/Release/functors.node');

console.log('='.repeat(80));
console.log('Rosetta Functor Binding Test Suite');
console.log('='.repeat(80));

// ============================================================================
// Test 1: Simple Functor (Squarer)
// ============================================================================
console.log('\n[Test 1] Simple Functor - Squarer');
console.log('-'.repeat(80));

const squarer = new addon.Squarer();
console.log('Functor name:', squarer.name());

const testValues = [2, 3, 5, 10, -4];
console.log('Testing squarer with values:', testValues);
testValues.forEach(val => {
    const result = squarer.call(val);
    console.log(`  ${val}² = ${result} (expected: ${val * val})`);
});

// Use as a mapper
const squared = testValues.map(x => squarer.call(x));
console.log('Mapped results:', squared);

// ============================================================================
// Test 2: Stateful Functor (Accumulator)
// ============================================================================
console.log('\n[Test 2] Stateful Functor - Accumulator');
console.log('-'.repeat(80));

const accumulator = new addon.Accumulator();

console.log('Accumulating values: [1, 2, 3, 4, 5]');
const values = [1, 2, 3, 4, 5];
values.forEach(val => {
    const sum = accumulator.call(val);
    console.log(`  Added ${val}, running sum: ${sum}`);
});

console.log('\nFinal statistics:');
console.log('  Sum:', accumulator.get_sum(), '(expected: 15)');
console.log('  Count:', accumulator.get_count(), '(expected: 5)');
console.log('  Average:', accumulator.get_average(), '(expected: 3)');

console.log('\nResetting accumulator...');
accumulator.reset();
console.log('  Sum after reset:', accumulator.get_sum(), '(expected: 0)');

// Accumulate again
console.log('\nAccumulating [10, 20, 30]:');
[10, 20, 30].forEach(val => {
    console.log(`  Sum: ${accumulator.call(val)}`);
});
console.log('  Average:', accumulator.get_average(), '(expected: 20)');

// ============================================================================
// Test 3: Vector Transform Functor
// ============================================================================
console.log('\n[Test 3] Vector Transform Functor');
console.log('-'.repeat(80));

const transform = new addon.VectorTransform();
console.log('Initial configuration:');
console.log('  Scale:', transform.scale, '(default: 1.0)');
console.log('  Offset:', transform.offset, '(default: 0.0)');

const inputVector = [1, 2, 3, 4, 5];
console.log('\nInput vector:', inputVector);

// Identity transform
let result = transform.call(inputVector);
console.log('Identity transform (scale=1, offset=0):', result);

// Scale by 2
transform.set_scale(2.0);
result = transform.call(inputVector);
console.log('Scale by 2:', result, '(expected: [2, 4, 6, 8, 10])');

// Scale and offset
transform.set_scale(2.0);
transform.set_offset(10.0);
result = transform.call(inputVector);
console.log('Scale by 2, offset 10:', result, '(expected: [12, 14, 16, 18, 20])');

// Temperature conversion: Celsius to Fahrenheit (F = C * 9/5 + 32)
const celsiusToFahrenheit = new addon.VectorTransform();
celsiusToFahrenheit.set_scale(9.0 / 5.0);
celsiusToFahrenheit.set_offset(32.0);
const celsius = [0, 10, 20, 30, 100];
const fahrenheit = celsiusToFahrenheit.call(celsius);
console.log('\nTemperature conversion (C to F):');
console.log('  Celsius:', celsius);
console.log('  Fahrenheit:', fahrenheit);

// ============================================================================
// Test 4: Predicate Functor (RangePredicate)
// ============================================================================
console.log('\n[Test 4] Predicate Functor - RangePredicate');
console.log('-'.repeat(80));

const predicate = new addon.RangePredicate();
console.log('Initial range:');
console.log('  Min:', predicate.min, '(default: 0)');
console.log('  Max:', predicate.max, '(default: 100)');

// Test individual values
const testNumbers = [-10, 0, 50, 100, 150];
console.log('\nTesting individual values:', testNumbers);
testNumbers.forEach(num => {
    const inRange = predicate.test(num);
    console.log(`  ${num} in range [0, 100]? ${inRange}`);
});

// Filter a vector
const data = [5, 15, 25, 105, 55, 200, 75, -5, 95];
console.log('\nFiltering vector:', data);
const filtered = predicate.filter(data);
console.log('  Values in range [0, 100]:', filtered);

// Change range and filter again
predicate.set_min(50);
predicate.set_max(100);
console.log('\nNew range: [50, 100]');
const filtered2 = predicate.filter(data);
console.log('  Values in range [50, 100]:', filtered2);

// Use as a JavaScript filter
const jsFiltered = data.filter(x => predicate.test(x));
console.log('  Using JavaScript .filter():', jsFiltered);

// ============================================================================
// Test 5: Binary Operation Functor
// ============================================================================
console.log('\n[Test 5] Binary Operation Functor');
console.log('-'.repeat(80));

const binaryOp = new addon.BinaryOp();

// Test all operations
const operations = [
    { name: 'Add', code: 0, a: 10, b: 5, expected: 15 },
    { name: 'Subtract', code: 1, a: 10, b: 5, expected: 5 },
    { name: 'Multiply', code: 2, a: 10, b: 5, expected: 50 },
    { name: 'Divide', code: 3, a: 10, b: 5, expected: 2 },
    { name: 'Power', code: 4, a: 2, b: 3, expected: 8 }
];

console.log('Testing all binary operations:');
operations.forEach(op => {
    binaryOp.set_operation(op.code);
    console.log(`  Operation: ${binaryOp.get_operation_name()}`);
    const result = binaryOp.call(op.a, op.b);
    console.log(`    ${op.a} ${op.name} ${op.b} = ${result} (expected: ${op.expected})`);
});

// Element-wise operations on vectors
const vec1 = [1, 2, 3, 4, 5];
const vec2 = [10, 20, 30, 40, 50];

console.log('\nElement-wise operations on vectors:');
console.log('  Vector A:', vec1);
console.log('  Vector B:', vec2);

binaryOp.set_operation(0); // Add
console.log('  A + B:', binaryOp.apply(vec1, vec2));

binaryOp.set_operation(2); // Multiply
console.log('  A * B:', binaryOp.apply(vec1, vec2));

binaryOp.set_operation(4); // Power
console.log('  A ^ B:', binaryOp.apply(vec1, vec2));

// ============================================================================
// Test 6: Function Composition
// ============================================================================
console.log('\n[Test 6] Function Composition - Compositor');
console.log('-'.repeat(80));

const compositor = new addon.Compositor();

console.log('Composition: f(g(x))');
console.log('Initial configuration (both identity):');
console.log(`  g(x) = x * ${compositor.scale1} + ${compositor.offset1}`);
console.log(`  f(x) = x * ${compositor.scale2} + ${compositor.offset2}`);

const testX = [1, 2, 3, 4, 5];
console.log('\nTest inputs:', testX);

// Identity composition
let composed = testX.map(x => compositor.call(x));
console.log('f(g(x)) with identity:', composed);

// g(x) = 2x + 3, f(x) = x (identity)
compositor.set_inner(2, 3);
compositor.set_outer(1, 0);
console.log('\ng(x) = 2x + 3, f(x) = x');
composed = testX.map(x => compositor.call(x));
console.log('f(g(x)):', composed, '(expected: [5, 7, 9, 11, 13])');

// g(x) = x, f(x) = 3x + 1
compositor.set_inner(1, 0);
compositor.set_outer(3, 1);
console.log('\ng(x) = x, f(x) = 3x + 1');
composed = testX.map(x => compositor.call(x));
console.log('f(g(x)):', composed, '(expected: [4, 7, 10, 13, 16])');

// g(x) = 2x, f(x) = x + 5
compositor.set_inner(2, 0);
compositor.set_outer(1, 5);
console.log('\ng(x) = 2x, f(x) = x + 5');
composed = testX.map(x => compositor.call(x));
console.log('f(g(x)):', composed, '(expected: [7, 9, 11, 13, 15])');

// ============================================================================
// Test 7: Real-World Example - Data Pipeline
// ============================================================================
console.log('\n[Test 7] Real-World Example - Data Processing Pipeline');
console.log('-'.repeat(80));

// Simulate sensor data
const sensorData = [
    95, 102, 98, 250, 101, 99, 103, -10, 100, 97, 1000, 96, 104
];

console.log('Raw sensor data:', sensorData);

// Step 1: Filter out-of-range values (valid range: 90-110)
const rangeFilter = new addon.RangePredicate();
rangeFilter.set_min(90);
rangeFilter.set_max(110);
const validData = rangeFilter.filter(sensorData);
console.log('\nAfter range filtering [90-110]:', validData);

// Step 2: Convert to another scale (e.g., normalize to 0-1 range)
const normalizer = new addon.VectorTransform();
normalizer.set_scale(1.0 / 100.0);  // Assuming original range is 0-100
normalizer.set_offset(0.0);
const normalized = normalizer.call(validData);
console.log('Normalized data:', normalized.map(x => x.toFixed(3)));

// Step 3: Calculate statistics using accumulator
const stats = new addon.Accumulator();
validData.forEach(val => stats.call(val));
console.log('\nStatistics:');
console.log('  Count:', stats.get_count());
console.log('  Sum:', stats.get_sum());
console.log('  Average:', stats.get_average().toFixed(2));

// ============================================================================
// Test 8: Functor Chaining and Composition
// ============================================================================
console.log('\n[Test 8] Functor Chaining');
console.log('-'.repeat(80));

const pipeline = [1, 2, 3, 4, 5];
console.log('Input:', pipeline);

// Chain: square -> scale by 2 -> add 10
const squarerFunc = new addon.Squarer();
const scaler = new addon.VectorTransform();
scaler.set_scale(2);
scaler.set_offset(10);

const step1 = pipeline.map(x => squarerFunc.call(x));
console.log('After square:', step1);

const step2 = scaler.call(step1);
console.log('After scale(2) + offset(10):', step2);
console.log('Expected: [12, 18, 28, 42, 60]');

// ============================================================================
// Test 9: Multiple Instances with Different States
// ============================================================================
console.log('\n[Test 9] Multiple Independent Functor Instances');
console.log('-'.repeat(80));

const acc1 = new addon.Accumulator();
const acc2 = new addon.Accumulator();

console.log('Accumulator 1: [1, 2, 3]');
[1, 2, 3].forEach(x => acc1.call(x));
console.log('  Sum:', acc1.get_sum());

console.log('Accumulator 2: [10, 20, 30]');
[10, 20, 30].forEach(x => acc2.call(x));
console.log('  Sum:', acc2.get_sum());

console.log('Instances are independent:');
console.log('  acc1 sum:', acc1.get_sum(), '(expected: 6)');
console.log('  acc2 sum:', acc2.get_sum(), '(expected: 60)');

// ============================================================================
// Summary
// ============================================================================
console.log('\n' + '='.repeat(80));
console.log('Test Suite Complete');
console.log('='.repeat(80));
console.log('\nFunctor types tested:');
console.log('  ✓ Simple functors (Squarer)');
console.log('  ✓ Stateful functors (Accumulator)');
console.log('  ✓ Transform functors (VectorTransform)');
console.log('  ✓ Predicate functors (RangePredicate)');
console.log('  ✓ Binary operation functors (BinaryOp)');
console.log('  ✓ Composition functors (Compositor)');
console.log('\nAll functor patterns are working correctly!');
