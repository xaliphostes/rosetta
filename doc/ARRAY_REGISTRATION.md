# std::array registration

## Main Features

1. Type-safe array conversion - Validates both size and element types
2. Support for type aliases - Like `Vec3 = std::array<double, 3>`
2. Common array types pre-registered - 2D/3D vectors, matrices, colors
2. Nested array support - For matrices represented as arrays of arrays
2. Compile-time size checking - Size is part of the type

## Key Differences from Vectors

- Fixed size - Size checked at compile-time and runtime
- Stack allocated - Better performance for small arrays
- Size validation - JavaScript arrays must match the exact size
- Type system - Size is part of the type (`array<int,3>` ≠ `array<int,4>`)

## Common Use Cases
The file includes examples for:

- 3D graphics (Vec3, Vec4, Mat4)
- Colors (RGB, RGBA)
- 2D games (Vec2, Rect)
- Physics (Force, Quaternion)
- Image processing (Pixel, Kernel)

## Usage Pattern
```cpp
// Define alias
using Vec3 = std::array<double, 3>;
REGISTER_TYPE_ALIAS_MANGLED(Vec3);

// Bind
rosetta::registerArrayAlias<Vec3, double, 3>(generator);
```
