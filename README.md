# Rosetta, a C++ automatic language binding

<p align="center">
  <img src="media/logo.png" alt="Logo rosetta" width="300">
</p>

<p align="center">
  <img src="https://img.shields.io/static/v1?label=Linux&logo=linux&logoColor=white&message=support&color=success" alt="Linux support">
  <img src="https://img.shields.io/static/v1?label=macOS&logo=apple&logoColor=white&message=support&color=success" alt="macOS support">
  <img src="https://img.shields.io/static/v1?label=Windows&logo=windows&logoColor=white&message=support&color=sucess" alt="Windows support">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20+-blue.svg" alt="Language">
  <img src="https://img.shields.io/badge/license-LGPL-blue.svg" alt="License">
</p>

# Rosetta

> **Non-intrusive C++ introspection library for automatic binding generation**

Rosetta enables seamless C++ to Python/JavaScript/Lua... bindings without modifying your classes. Register once, export everywhere.

## Features

- ✨ **Zero-intrusion**: No inheritance, no macros in your classes
- 🐍 **Automatic Python bindings** via pybind11
- 🌐 **JavaScript binding** via NAPI
- 🌐 **WASM bindings** via Emscripten
- 📦 **Container support**: `std::vector`, `std::array`, `std::map`, `std::optional`, etc.
- 🎯 **Smart pointers**: `shared_ptr`, `unique_ptr`, raw pointers
- 🏛️ **Full inheritance**: Virtual methods, abstract classes, multiple inheritance
- 🔍 **Const correctness**: Distinguishes const/non-const methods
- 📝 **Auto-documentation**: Generate Markdown/HTML docs
- ✅ **Validation**: Constraint system for runtime checks

## Quick Start

### 1. Define Your Classes (No Changes Needed!)

```cpp
class Vector3D {
public:
    double x, y, z;
    
    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    void normalize();
};
```

### 2. Register with Rosetta

```cpp
#include "rosetta/rosetta.hpp"

void register_types() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}
```
That way, any scripting language can be binded using this Rosetta instrospection.

### 3. Generate Python Bindings

```cpp
#include <pybind11/pybind11.h>
#include "rosetta/generators/python_binding_generator.hpp"

PYBIND11_MODULE(my_module, m) {
    register_types();
    
    rosetta::generators::PythonBindingGenerator(m)
        .bind_class<Vector3D>();
}
```

### 4. Use in Python

```python
import my_module

v = my_module.Vector3D()
v.x = 3.0
v.y = 4.0
v.z = 0.0
print(v.length())  # 5.0
v.normalize()
```

### 5. Generate Javascript Bindings
```cpp
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::generators::JavaScriptBindingGenerator(env, exports)
      .bind_class<Vector3D>();

    return exports;
}

NODE_API_MODULE(my_module, Init)
```

### 6. Use in JavaScript

```js
const my_module = require('./build/Release/my_module')

v = new my_module.Vector3D()
v.x = 3.0
v.y = 4.0
v.z = 0.0
console.log(v.length())  # 5.0
v.normalize()
```

## Advanced Features

### Inheritance

```cpp
class Shape {
public:
    virtual double area() const = 0;
};

class Circle : public Shape {
public:
    double radius;
    double area() const override;
};

ROSETTA_REGISTER_CLASS(Shape)
    .pure_virtual_method<double>("area");

ROSETTA_REGISTER_CLASS(Circle)
    .inherits_from<Shape>("Shape")
    .field("radius", &Circle::radius)
    .override_method("area", &Circle::area);
```

### Validation

```cpp
using namespace rosetta;

ConstraintValidator::instance()
    .add_field_constraint<Circle, double>(
        "radius",
        make_range_constraint(0.0, 1000.0)
    );

Circle c;
c.radius = -5.0;

std::vector<std::string> errors;
if (!ConstraintValidator::instance().validate(c, errors)) {
    for (const auto& err : errors) {
        std::cerr << err << "\n";  // "radius: Value must be between 0 and 1000"
    }
}
```

### Multiple Bindings

```cpp
// Python
rosetta::PythonGenerator py_gen;
std::cout << py_gen.generate();

// JavaScript
rosetta::JavaScriptGenerator js_gen;
std::cout << js_gen.generate();

// TypeScript definitions
rosetta::TypeScriptGenerator ts_gen;
std::cout << ts_gen.generate();

// Documentation
rosetta::DocGenerator doc_gen;
std::cout << doc_gen.generate();
```

## Installation

### Requirements

- C++20 or later
- CMake 3.15+
- Optional: pybind11 (for Python bindings)
- Optional: NAPI (for JavaScript bindings)
- Optional: emscripten (for JavaScript bindings)

### Build

```bash
git clone https://github.com/yourusername/rosetta.git
cd rosetta
mkdir build && cd build
cmake ..
make
```

### Include in Your Project

```cmake
add_subdirectory(rosetta)
target_link_libraries(your_target rosetta::rosetta)
```

Or header-only:

```cpp
#include "rosetta/rosetta.hpp"
```

## Architecture

```
rosetta/
├── core/              # Core introspection engine
├── traits/            # Type detection (containers, pointers, inheritance)
├── generators/        # Binding generators (Python, JS, TS)
└── extensions/        # Optional features (serialization, validation, docs)
```

## Examples

See `examples/` directory for complete working examples:

- `examples/basic/` - Simple class registration
- `examples/inheritance/` - Virtual methods and polymorphism
- `examples/python/` - Full Python binding
- `examples/validation/` - Constraint validation

## Comparison

| Feature | Rosetta | Manual pybind11 | SWIG | Boost.Python |
|---------|---------|----------------|------|--------------|
| **Non-intrusive** | ✅ | ✅ | ✅ | ❌ |
| **Modern C++17** | ✅ | ✅ | ❌ | ⚠️ |
| **Zero boilerplate** | ✅ | ❌ | ❌ | ❌ |
| **Type-safe** | ✅ | ✅ | ⚠️ | ✅ |
| **Multiple targets** | ✅ | ❌ | ✅ | ❌ |
| **Compile-time** | ✅ | ✅ | ❌ | ✅ |

## Limitations

- Requires explicit registration (not fully automatic)
- Template classes need per-instantiation registration
- Operator overloading requires manual declaration

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) first.

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Credits

Created by [Your Name](https://github.com/yourusername)

Inspired by:
- [pybind11](https://github.com/pybind/pybind11) - Python bindings
- [Boost.PFR](https://github.com/boostorg/pfr) - Reflection for aggregates
- [rttr](https://github.com/rttrorg/rttr) - Runtime reflection

## Support

- 📖 [Documentation](https://rosetta.readthedocs.io)
- 💬 [Discord](https://discord.gg/rosetta)
- 🐛 [Issue Tracker](https://github.com/yourusername/rosetta/issues)
- ⭐ Star us on GitHub!

---

**One registration, infinite possibilities** 🚀