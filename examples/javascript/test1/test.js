const geometry = require('./build/Release/geometry');

// ============================================================================
// Classes
// ============================================================================

// Create vectors
const v1 = new geometry.Vector3D();
v1.x = 3.0;
v1.y = 4.0;
v1.z = 0.0;

console.log('Length:', v1.length());  // 5.0
console.log('String:', v1.to_string()); // "Vector3D(3.0, 4.0, 0.0)"

// Normalize
v1.normalize();
console.log('Normalized:', v1.x, v1.y, v1.z);  // 0.6, 0.8, 0.0

// Vector operations
const v2 = new geometry.Vector3D();
v2.x = 1.0;
v2.y = 2.0;
v2.z = 3.0;

// const v3 = v1.add(v2);
// console.log('Sum:', v3.x, v3.y, v3.z);

// const v4 = v1.scale(2.0);
// console.log('Scaled:', v4.x, v4.y, v4.z);

// Shapes
const circle = new geometry.Circle();
circle.radius = 5.0;
console.log('Circle area:', circle.area());        // 78.5398
console.log('Circle perimeter:', circle.perimeter()); // 31.4159
console.log('Circle type:', circle.type());        // "Circle"

const rect = new geometry.Rectangle();
rect.width = 10.0;
rect.height = 5.0;
console.log('Rectangle area:', rect.area());        // 50.0
console.log('Rectangle perimeter:', rect.perimeter()); // 30.0

// ============================================================================
// Global Functions
// ============================================================================

// Calculate distance
const distance = geometry.calculateDistance(v1, v2);
console.log('Distance:', distance);

// Generate sequence
const sequence = geometry.generateSequence(10, 0.0, 1.5);
console.log('Sequence:', sequence);
// [0.0, 1.5, 3.0, 4.5, 6.0, 7.5, 9.0, 10.5, 12.0, 13.5]

// Get statistics
const data = [1.0, 2.0, 3.0, 4.0, 5.0];
const stats = geometry.getStatistics(data);
console.log('Statistics:', stats);
// { min: 1, max: 5, mean: 3, sum: 15, count: 5 }

// ============================================================================
// Callbacks (Functors)
// ============================================================================

// Apply function (C++ calls JS callback)
geometry.applyFunction([1, 2, 3, 4, 5], (value) => {
    console.log('Value:', value);
});
// Output: Value: 1, Value: 2, Value: 3, Value: 4, Value: 5

// Transform values (C++ calls JS callback and returns result)
const doubled = geometry.transformValues([1, 2, 3, 4, 5], (x) => x * 2);
console.log('Doubled:', doubled);
// [2, 4, 6, 8, 10]

const squared = geometry.transformValues([1, 2, 3, 4, 5], (x) => x * x);
console.log('Squared:', squared);
// [1, 4, 9, 16, 25]

// Complex transformations
const result = geometry.transformValues([1, 2, 3, 4, 5], (x) => {
    return Math.sin(x) * 10;
});
console.log('Sin*10:', result);

// ============================================================================
// Enumerations
// ============================================================================

console.log('Colors:');
console.log('  Red:', geometry.Color.Red);       // 0
console.log('  Green:', geometry.Color.Green);   // 1
console.log('  Blue:', geometry.Color.Blue);     // 2

console.log('ShapeTypes:');
console.log('  Circle:', geometry.ShapeType.Circle);      // 0
console.log('  Rectangle:', geometry.ShapeType.Rectangle); // 1

// ============================================================================
// Arrays and Maps
// ============================================================================

// Arrays (std::vector) work seamlessly
const numbers = [1.0, 2.0, 3.0, 4.0, 5.0];
const moreStats = geometry.getStatistics(numbers);

// Maps (std::map) are converted to/from JS objects
const settings = {
    "temperature": 25.5,
    "humidity": 65.0,
    "pressure": 1013.25
};
// Can be passed to C++ functions expecting std::map<std::string, double>