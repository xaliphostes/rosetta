# Quick Start: Rosetta N-API Runtime Bindings

## What is This?

This is a **runtime** JavaScript binding generator that uses Rosetta's introspection to create N-API bindings **without code generation**. Just register your C++ classes with Rosetta, then expose them to JavaScript with one line!

## The Magic ✨

Instead of generating code like other tools (SWIG, etc.), this system:
1. Uses Rosetta's runtime introspection metadata
2. Creates N-API bindings on-the-fly when the module loads
3. Requires **zero** boilerplate per class

## Example

### Traditional Approach (SWIG, NAN, etc.)

```cpp
// Write wrapper code for EACH class
// 100+ lines of boilerplate per class
%module example
%{
#include "Vector3D.h"
%}

%include "Vector3D.h"
// ... tons of configuration ...
```

### Rosetta Approach

```cpp
// Register once with Rosetta
ROSETTA_REGISTER_CLASS(Vector3D)
    .field("x", &Vector3D::x)
    .method("length", &Vector3D::length);

// Bind to JavaScript - ONE LINE!
BEGIN_NAPI_MODULE(geometry) {
    NapiBindingGenerator(env, exports).bind_class<Vector3D>();
    return exports;
}
END_NAPI_MODULE(geometry)
```

## Complete Minimal Example

### 1. Your C++ Code (my_module.cpp)

```cpp
#include "napi_binding_generator.h"
#include "rosetta.h"

// Your existing C++ class - NO MODIFICATIONS NEEDED
class Vector3D {
public:
    double x, y, z;
    
    Vector3D(double x = 0, double y = 0, double z = 0) 
        : x(x), y(y), z(z) {}
    
    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

// Register with Rosetta (do this once at startup)
void init() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length);
}

// Create the Node.js module
BEGIN_NAPI_MODULE(my_module) {
    init();  // Register classes
    
    // Bind to JavaScript - that's it!
    rosetta::generators::NapiBindingGenerator(env, exports)
        .bind_class<Vector3D>();
    
    return exports;
}
END_NAPI_MODULE(my_module)
```

### 2. Build (package.json)

```json
{
  "name": "my_module",
  "version": "1.0.0",
  "main": "index.js",
  "scripts": {
    "install": "node-gyp rebuild"
  },
  "dependencies": {
    "node-addon-api": "^7.0.0"
  }
}
```

### 3. Configure (binding.gyp)

```json
{
  "targets": [{
    "target_name": "my_module",
    "sources": ["my_module.cpp"],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    "dependencies": [
      "<!(node -p \"require('node-addon-api').gyp\")"
    ],
    "cflags_cc": ["-std=c++17"],
    "defines": ["NAPI_CPP_EXCEPTIONS"]
  }]
}
```

### 4. Build & Use

```bash
# Install dependencies
npm install

# Build (automatic via package.json)
# Or manually: node-gyp configure build
```

### 5. JavaScript Usage

```javascript
const myModule = require('./build/Release/my_module');

const vec = new myModule.Vector3D();
vec.x = 3.0;
vec.y = 4.0;
vec.z = 0.0;

console.log(vec.length());  // 5.0
```

## Advanced: Multiple Classes

```cpp
BEGIN_NAPI_MODULE(geometry) {
    init_rosetta();
    
    // Bind many classes at once!
    rosetta::generators::NapiBindingGenerator(env, exports)
        .bind_classes<Vector3D, Matrix4x4, Quaternion, Transform>()
        .bind_function(calculate_distance, "calculateDistance")
        .bind_enum<Color>("Color", {
            {"Red", Color::Red},
            {"Green", Color::Green}
        });
    
    return exports;
}
END_NAPI_MODULE(geometry)
```

## Advanced: Callbacks (Functors)

```cpp
// C++ function that takes a callback
std::vector<double> transform(const std::vector<double>& data,
                              std::function<double(double)> func) {
    std::vector<double> result;
    for (double val : data) {
        result.push_back(func(val));
    }
    return result;
}

// Bind it
generator.bind_function(transform, "transform");
```

```javascript
// Use it in JavaScript
const doubled = myModule.transform([1, 2, 3, 4, 5], (x) => x * 2);
console.log(doubled);  // [2, 4, 6, 8, 10]
```

## What Gets Converted Automatically?

- ✅ **Primitives**: `bool`, `int`, `double`, `std::string`
- ✅ **Containers**: `std::vector`, `std::array`, `std::map`, `std::optional`
- ✅ **Functions**: `std::function<Ret(Args...)>` (both directions!)
- ✅ **Classes**: Any class registered with Rosetta
- ✅ **Enums**: Any C++ enum

## Files in This Package

- **`napi_binding_generator.h`** - Main runtime binding generator
- **`napi_type_converter.h`** - Type conversion system
- **`napi_binding_example.cpp`** - Complete working example
- **`binding.gyp`** - Build configuration
- **`package.json`** - npm package configuration
- **`README.md`** - Full documentation

## Why This is Better

| Feature | Rosetta Runtime | Traditional Bindings |
|---------|----------------|---------------------|
| Lines of code per class | **1** | 50-200 |
| Code generation needed | **No** | Yes |
| Supports callbacks | **Yes** | Limited |
| Automatic containers | **Yes** | Manual |
| Inheritance support | **Yes** | Manual |
| Type safety | **Full** | Partial |
| Maintenance | **Low** | High |

## Next Steps

1. **See full example**: Check `napi_binding_example.cpp` for a complete working demo
2. **Read full docs**: See `README.md` for all features
3. **Try it**: Build the example and test in JavaScript!

## Requirements

- Node.js 16+
- C++17 compiler
- node-addon-api 7.0+
- Rosetta library

## Support

For questions or issues:
- Check the full README.md
- Review the example code
- Look at the inline documentation

---

**That's it!** One line to bind a class. No code generation. Pure runtime magic. ✨