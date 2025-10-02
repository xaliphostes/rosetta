# Complete Auto Vector Registration Guide

## Overview

The enhanced type registry now automatically handles vector types with mangled names, eliminating the need to manually specify type names like `"NSt3__16vectorIdNS_9allocatorIdEEEE"`.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  TypeNameRegistry (Singleton)                           │
│  - Automatically registers common vector types at init │
│  - Maps typeid(T).name() → type name                   │
└─────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│  JavaScript/Python/Lua Generators                       │
│  - Use js_vector_helpers.h for automatic converters    │
│  - Type names match via typeid().name()                │
└─────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Basic Vector Types (Auto-Registered)

These work automatically without any registration:

```cpp
#include <rosetta/rosetta.h>

class MyClass : public rosetta::Introspectable {
    INTROSPECTABLE(MyClass)
public:
    std::vector<double> getData() const { return data_; }
    void setData(const std::vector<double>& d) { data_ = d; }
    
private:
    std::vector<double> data_;  // ✓ Auto-registered
};

void MyClass::registerIntrospection(rosetta::TypeRegistrar<MyClass> reg) {
    reg.member("data", &MyClass::data_)
       .method("getData", &MyClass::getData)
       .method("setData", &MyClass::setData);
}
```

**Auto-registered types:**
- `vector<int>`, `vector<unsigned int>`
- `vector<int32_t>`, `vector<uint32_t>`
- `vector<int64_t>`, `vector<uint64_t>`, `vector<size_t>`
- `vector<float>`, `vector<double>`
- `vector<bool>`, `vector<char>`, `vector<string>`

### 2. Type Aliases

For type aliases, add one macro:

```cpp
#include <rosetta/rosetta.h>

// Define your aliases
using Vertices = std::vector<double>;
using Triangles = std::vector<size_t>;
using Names = std::vector<std::string>;

// Register them (use their mangled names automatically)
REGISTER_TYPE_ALIAS_MANGLED(Vertices);
REGISTER_TYPE_ALIAS_MANGLED(Triangles);
REGISTER_TYPE_ALIAS_MANGLED(Names);

class Surface : public rosetta::Introspectable {
    INTROSPECTABLE(Surface)
public:
    Surface() {}
    Surface(const Vertices& v, const Triangles& t)
        : vertices_(v), triangles_(t) {}

    const Vertices& vertices() const { return vertices_; }
    const Triangles& triangles() const { return triangles_; }

private:
    Vertices vertices_;
    Triangles triangles_;
};

void Surface::registerIntrospection(rosetta::TypeRegistrar<Surface> reg) {
    reg.constructor<>()
       .constructor<const Vertices&, const Triangles&>()
       .method("vertices", &Surface::vertices)
       .method("triangles", &Surface::triangles);
}
```

### 3. JavaScript Binding (Simple)

```cpp
#include <rosetta/generators/js.h>
#include <rosetta/generators/js_vector_helpers.h>

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    
    // Register type alias converters
    rosetta::registerTypeAlias<Vertices, double>(generator);
    rosetta::registerTypeAlias<Triangles, size_t>(generator);
    
    // Bind class
    generator.bind_class<Surface>();
    
    return exports;
}

NODE_API_MODULE(surface, Init)
```

**JavaScript Usage:**
```javascript
const addon = require('./build/Release/surface');

const s = new addon.Surface(
    [0, 1, 2, 3, 4, 5, 6, 7, 8],  // Vertices
    [0, 1, 2]                      // Triangles
);

console.log(s.vertices());  // [0, 1, 2, 3, 4, 5, 6, 7, 8]
console.log(s.triangles()); // [0, 1, 2]
```

### 4. Python Binding (Coming Soon)

Similar pattern for Python - need to add vector helpers:

```cpp
#include <rosetta/generators/py.h>
#include <rosetta/generators/py_vector_helpers.h>

PYBIND11_MODULE(surface, m) {
    rosetta::PyGenerator generator(m);
    
    // Register vector converters
    rosetta::registerTypeAlias<Vertices, double>(generator);
    rosetta::registerTypeAlias<Triangles, size_t>(generator);
    
    generator.bind_class<Surface>();
}
```

## Advanced Usage

### Custom Vector Types

For vectors of custom types:

```cpp
struct Point3D {
    double x, y, z;
};
REGISTER_TYPE(Point3D);

using Points = std::vector<Point3D>;
REGISTER_TYPE_ALIAS_MANGLED(Points);

// In JavaScript binding:
rosetta::registerTypeAlias<Points, Point3D>(generator);

// You'll need a custom converter for Point3D:
generator.register_type_converter(
    typeid(Point3D).name(),
    // C++ to JS
    [](Napi::Env env, const std::any& value) -> Napi::Value {
        auto p = std::any_cast<Point3D>(value);
        auto obj = Napi::Object::New(env);
        obj.Set("x", p.x);
        obj.Set("y", p.y);
        obj.Set("z", p.z);
        return obj;
    },
    // JS to C++
    [](const Napi::Value& js_val) -> std::any {
        auto obj = js_val.As<Napi::Object>();
        return Point3D{
            obj.Get("x").As<Napi::Number>().DoubleValue(),
            obj.Get("y").As<Napi::Number>().DoubleValue(),
            obj.Get("z").As<Napi::Number>().DoubleValue()
        };
    }
);
```

### Batch Registration

Register many types at once:

```cpp
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    
    // Option 1: Register all common types
    rosetta::registerCommonVectorTypes(generator);
    
    // Option 2: Register specific types
    rosetta::registerVectorType<double>(generator);
    rosetta::registerVectorType<size_t>(generator);
    rosetta::registerVectorType<uint32_t>(generator);
    
    // Then register your type aliases
    rosetta::registerTypeAlias<Vertices, double>(generator);
    rosetta::registerTypeAlias<Triangles, size_t>(generator);
    
    // Bind all classes
    generator.bind_classes<Surface, Mesh, Model>();
    
    return exports;
}
```

## Debugging

### Check Registered Types

```cpp
#include <rosetta/type_registry.h>

void debug_types() {
    auto& registry = rosetta::TypeNameRegistry::instance();
    
    // Check if type is registered
    std::cout << "vector<double> registered: " 
              << registry.is_registered<std::vector<double>>() 
              << "\n";
    
    // Get type name
    std::cout << "vector<double> name: " 
              << registry.get_name<std::vector<double>>() 
              << "\n";
    
    // List all registered types
    for (const auto& name : registry.get_all_registered_types()) {
        std::cout << "  - " << name << "\n";
    }
}
```

### Common Issues

**Issue 1: "bad any cast" error**
```
Error: Constructor failed: bad any cast
```

**Solution:** Register the type alias:
```cpp
REGISTER_TYPE_ALIAS_MANGLED(Vertices);
rosetta::registerTypeAlias<Vertices, double>(generator);
```

**Issue 2: "Unsupported type" error**
```
Error: Constructor failed: Unsupported type: Vertices
```

**Solution:** The converter isn't registered. Add to Init():
```cpp
rosetta::registerTypeAlias<Vertices, double>(generator);
```

**Issue 3: Type name mismatch**

Print the actual type names to debug:
```cpp
std::cout << "Expected: " << typeid(Vertices).name() << "\n";
std::cout << "Registered: " << registry.get_name<Vertices>() << "\n";
```

## File Structure

```
include/rosetta/
├── rosetta.h                          # Main header
├── type_registry.h                     # ✨ Enhanced with auto-registration
├── inline/
│   └── type_registry.hxx              # ✨ Auto-registers common vectors
└── generators/
    ├── js.h                           # JavaScript generator
    ├── js_vector_helpers.h            # ✨ NEW: Auto vector converters
    ├── py.h                           # Python generator
    └── inline/
        ├── js.hxx
        └── py.hxx

examples/scripting/
├── js3/
│   ├── binding.cxx                    # ✨ Updated example
│   ├── test.js
│   └── CMakeLists.txt
└── python/
    └── binding.cxx
```

## Migration Guide

### Before (Manual Names)

```cpp
// Old way - manual mangled names
generator.register_type_converter(
    "NSt3__16vectorIdNS_9allocatorIdEEEE",  // 😱 Hard to read
    /* converters */
);
```

### After (Automatic)

```cpp
// New way - automatic
REGISTER_TYPE_ALIAS_MANGLED(Vertices);
rosetta::registerTypeAlias<Vertices, double>(generator);
```

## Performance

- **Compile time:** Negligible overhead (static initialization)
- **Runtime:** O(1) hash table lookup
- **Memory:** ~50 bytes per registered type
- **Startup:** All common types registered once at program start

## Best Practices

1. **Use type aliases for semantics:**
   ```cpp
   using Vertices = std::vector<double>;  // ✓ Clear intent
   // vs
   std::vector<double> vertices;          // ✗ Less clear
   ```

2. **Register aliases early:**
   ```cpp
   // In your header, right after typedef
   using Vertices = std::vector<double>;
   REGISTER_TYPE_ALIAS_MANGLED(Vertices);
   ```

3. **Group related registrations:**
   ```cpp
   // geometry_types.h
   using Points = std::vector<Point3D>;
   using Vertices = std::vector<double>;
   using Triangles = std::vector<size_t>;
   
   REGISTER_TYPE_ALIAS_MANGLED(Points);
   REGISTER_TYPE_ALIAS_MANGLED(Vertices);
   REGISTER_TYPE_ALIAS_MANGLED(Triangles);
   ```

4. **Use helper functions for complex setups:**
   ```cpp
   void registerGeometryTypes(rosetta::JsGenerator& gen) {
       rosetta::registerTypeAlias<Points, Point3D>(gen);
       rosetta::registerTypeAlias<Vertices, double>(gen);
       rosetta::registerTypeAlias<Triangles, size_t>(gen);
   }
   ```

## Next Steps

- [ ] Add Python vector helpers (`py_vector_helpers.h`)
- [ ] Add Lua vector helpers (`lua_vector_helpers.h`)
- [ ] Support nested vectors (`vector<vector<T>>`)
- [ ] Add array type support (`std::array<T, N>`)
- [ ] Add map type support (`std::map<K, V>`)