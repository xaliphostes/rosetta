// ============================================================================
// Simple examples showing how to use C++ functors from JavaScript
// ============================================================================

const addon = require('./build/Release/functors.node');

console.log('Functor Examples\n');

// ============================================================================
// Example 1: Using a Simple Functor
// ============================================================================
if (1) {
    console.log('=== Example 1: Simple Functor ===');
    const squarer = new addon.Squarer();

    // Call the functor like a function
    console.log('Square of 5:', squarer.call(5));        // 25
    console.log('Square of 10:', squarer.call(10));      // 100

    // Use with array methods
    const numbers = [1, 2, 3, 4, 5];
    const squares = numbers.map(x => squarer.call(x));
    console.log('Squares:', squares);                     // [1, 4, 9, 16, 25]
}

// ============================================================================
// Example 2: Stateful Functor
// ============================================================================
if (1) {
    console.log('\n=== Example 2: Stateful Functor ===');
    const accumulator = new addon.Accumulator();

    // The functor maintains state between calls
    console.log('Add 5:', accumulator.call(5));           // 5
    console.log('Add 10:', accumulator.call(10));         // 15
    console.log('Add 15:', accumulator.call(15));         // 30

    console.log('Total sum:', accumulator.get_sum());     // 30
    console.log('Count:', accumulator.get_count());       // 3
    console.log('Average:', accumulator.get_average());   // 10
}

// ============================================================================
// Example 3: Configurable Functor
// ============================================================================
if (1) {
    console.log('\n=== Example 3: Configurable Transform ===');
    const transform = new addon.VectorTransform();

    // Configure the transformation: y = x * scale + offset
    transform.set_scale(2.0);
    transform.set_offset(3.0);

    const input = [1, 2, 3, 4, 5];
    const output = transform.call(input);
    console.log('Input:', input);
    console.log('Output (2x + 3):', output);              // [5, 7, 9, 11, 13]
}

// ============================================================================
// Example 4: Predicate Functor (Filtering)
// ============================================================================
if (1) {
    console.log('\n=== Example 4: Predicate for Filtering ===');
    const predicate = new addon.RangePredicate();
    predicate.set_min(10);
    predicate.set_max(50);

    const data = [5, 15, 25, 75, 35, 100, 45];
    console.log('Data:', data);

    // Test individual values
    console.log('Is 25 in range?', predicate.test(25));   // true
    console.log('Is 75 in range?', predicate.test(75));   // false

    // Filter entire array
    const filtered = predicate.filter(data);
    console.log('Filtered [10-50]:', filtered);           // [15, 25, 35, 45]

    // Use with JavaScript filter
    const jsFiltered = data.filter(x => predicate.test(x));
    console.log('JS filtered:', jsFiltered);              // [15, 25, 35, 45]
}

// ============================================================================
// Example 5: Binary Operations
// ============================================================================
if (1) {
    console.log('\n=== Example 5: Binary Operations ===');
    const binaryOp = new addon.BinaryOp();

    // Addition
    binaryOp.set_operation(0);  // 0 = Add
    console.log('10 + 5 =', binaryOp.call(10, 5));       // 15

    // Multiplication  
    binaryOp.set_operation(2);  // 2 = Multiply
    console.log('10 * 5 =', binaryOp.call(10, 5));       // 50

    // Element-wise operations on arrays
    const a = [1, 2, 3, 4, 5];
    const b = [10, 20, 30, 40, 50];
    console.log('Element-wise multiply:');
    console.log('  A:', a);
    console.log('  B:', b);
    console.log('  A * B:', binaryOp.apply(a, b));       // [10, 40, 90, 160, 250]
}

// ============================================================================
// Example 6: Real-World - Data Processing Pipeline
// ============================================================================
if (0) {
    console.log('\n=== Example 6: Data Processing Pipeline ===');

    // Raw sensor readings
    const sensors = [95, 102, 300, 98, 101, -20, 99, 103];
    console.log('Raw sensor data:', sensors);

    // Step 1: Filter valid range (90-110)
    const validator = new addon.RangePredicate();
    validator.set_min(90);
    validator.set_max(110);
    const valid = validator.filter(sensors);
    console.log('Valid readings:', valid);                // [95, 102, 98, 101, 99, 103]

    // Step 2: Normalize (scale to 0-1 range)
    const normalizer = new addon.VectorTransform();
    normalizer.set_scale(1.0 / 100.0);
    const normalized = normalizer.call(valid);
    console.log('Normalized:', normalized.map(x => x.toFixed(2)));

    // Step 3: Calculate statistics
    const stats = new addon.Accumulator();
    valid.forEach(val => stats.call(val)); // !!!!!!!!!!!!!!!!!!!! failure here <----------------
    console.log('Average reading:', stats.get_average().toFixed(2));
}

// ============================================================================
// Example 7: Creating Multiple Independent Functors
// ============================================================================
if (0) {
    console.log('\n=== Example 7: Multiple Independent Instances ===');

    // Create two accumulators for different data streams
    const stream1 = new addon.Accumulator();
    const stream2 = new addon.Accumulator();

    [1, 2, 3].forEach(x => stream1.call(x));
    [100, 200, 300].forEach(x => stream2.call(x));

    console.log('Stream 1 sum:', stream1.get_sum());      // 6
    console.log('Stream 2 sum:', stream2.get_sum());      // 600
    console.log('Both functors maintain independent state!');

    console.log('\n✓ All examples complete!');
}
