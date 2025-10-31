# Rosetta Python Bindings

Automatic Python bindings for C++ classes using Rosetta introspection and pybind11.

## Features

- **Zero-intrusion**: No modifications needed to your C++ classes
- **Runtime binding generation**: Uses Rosetta's introspection at runtime
- **Type-safe**: Automatic type conversion with fallbacks
- **STL support**: Built-in converters for vectors, maps, optionals, etc.
- **Extensible**: Easy to add custom type converters
- **Pythonic**: Generates idiomatic Python APIs

## Quick Start

### 1. Register Your C++ Classes with Rosetta

```cpp
#include "rosetta.h"

class Vector3D {
public:
    double x, y, z;
    double length() const;
    void normalize();
};

void register_classes() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}
```

### 2. Create Python Bindings

```cpp
#include "py_generator.h"

PYBIND11_MODULE(mymodule, m) {
    m.doc() = "My C++ module";
    
    register_classes();
    
    rosetta::generators::python::PyGenerator gen(m);
    gen.bind_class<Vector3D>("Vector3D");
}
```

### 3. Use in Python

```python
import mymodule

v = mymodule.Vector3D()
v.x = 3.0
v.y = 4.0
v.z = 0.0

print(f"Length: {v.length()}")  # Output: Length: 5.0
v.normalize()
```

## Installation

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
make install
```

### Using setup.py

```bash
pip install .
```

For development:

```bash
pip install -e ".[dev]"
```

## Advanced Usage

### Binding Multiple Classes

```cpp
// Using variadic templates
gen.bind_classes<Vector3D, Circle, Rectangle, Person>();

// Or using the macro
PY_BIND_CLASSES(gen, Vector3D, Circle, Rectangle, Person);
```

### Binding Free Functions

```cpp
double distance(const Vector3D& a, const Vector3D& b) {
    // implementation
}

gen.bind_function("distance", &distance, "Calculate distance between vectors");
```

### Custom Type Converters

```cpp
struct Color {
    unsigned char r, g, b, a;
};

// Convert Color to Python tuple
gen.register_converter<Color>(
    [](const rosetta::core::Any& val) -> py::object {
        const Color& c = val.as<Color>();
        return py::make_tuple(c.r, c.g, c.b, c.a);
    },
    [](const py::object& val) -> rosetta::core::Any {
        auto seq = val.cast<py::sequence>();
        return rosetta::core::Any(Color{
            seq[0].cast<unsigned char>(),
            seq[1].cast<unsigned char>(),
            seq[2].cast<unsigned char>(),
            seq[3].cast<unsigned char>()
        });
    }
);
```

### Using Common Type Converters

```cpp
#include "py_type_converters.h"

// Register all common STL converters
rosetta::generators::python::register_common_converters(gen);

// Or register specific converters
register_vector_converter<double>(gen);
register_map_converter<std::string, int>(gen);
register_optional_converter<std::string>(gen);
```

### Inheritance Support

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

// Register with inheritance
ROSETTA_REGISTER_CLASS(Shape)
    .pure_virtual_method<double>("area");

ROSETTA_REGISTER_CLASS(Circle)
    .inherits_from<Shape>()
    .field("radius", &Circle::radius)
    .override_method("area", &Circle::area);

// Bind to Python
gen.bind_classes<Shape, Circle>();
```

## Supported Types

### Basic Types
- `int`, `long`, `float`, `double`, `bool`
- `std::string`

### STL Containers
- `std::vector<T>`
- `std::array<T, N>`
- `std::map<K, V>`
- `std::unordered_map<K, V>`
- `std::set<T>`
- `std::pair<T1, T2>`

### Smart Pointers
- `std::shared_ptr<T>`
- `std::unique_ptr<T>` (limited support)

### Optional
- `std::optional<T>`

## Macros

### BEGIN_PY_MODULE / END_PY_MODULE

Simplify module creation:

```cpp
BEGIN_PY_MODULE(mymodule, "Module documentation") {
    register_classes();
    gen.bind_classes<Vector3D, Person>();
    gen.add_utilities();
}
END_PY_MODULE();
```

### PY_BIND_CLASS

Bind a single class:

```cpp
PY_BIND_CLASS(gen, Vector3D);
```

### PY_BIND_CLASSES

Bind multiple classes:

```cpp
PY_BIND_CLASSES(gen, Vector3D, Circle, Rectangle);
```

### PY_BIND_FUNCTION

Bind a free function:

```cpp
PY_BIND_FUNCTION(gen, distance, "Calculate distance");
```

## Utilities

The generator provides utility functions:

```python
import mymodule

# Get all registered classes
classes = mymodule.list_classes()
print(f"Available classes: {classes}")

# Get version
version = mymodule.get_version()
print(f"Version: {version}")
```

## Type Conversion

### Automatic Conversions

The generator automatically handles conversions between:
- C++ numeric types ↔ Python `int`/`float`
- `std::string` ↔ Python `str`
- `std::vector<T>` ↔ Python `list`
- `std::map<K,V>` ↔ Python `dict`
- `std::optional<T>` ↔ Python `None` or value
- `bool` ↔ Python `bool`

### Numeric Type Flexibility

The `Any` class supports automatic numeric conversions:

```cpp
// C++ side: field is double
double value = 3.14;

// Python side: can assign int
obj.value = 42  # Automatically converted to double
```

## Best Practices

### 1. Register Classes First

Always register classes with Rosetta before binding:

```cpp
register_classes();  // Register with Rosetta
gen.bind_classes<...>();  // Bind to Python
```

### 2. Use Common Converters

Register common type converters at the start:

```cpp
rosetta::generators::python::register_common_converters(gen);
```

### 3. Provide Documentation

Add docstrings to functions and classes:

```cpp
gen.bind_function("distance", &distance, 
    "Calculate Euclidean distance between two vectors");
```

### 4. Handle Exceptions

The generator automatically converts C++ exceptions to Python exceptions.

### 5. Memory Management

- Use `std::shared_ptr` for shared ownership
- Return values by value when possible
- Avoid returning raw pointers

## Comparison with Other Binding Tools

| Feature | Rosetta + pybind11 | pybind11 alone | Boost.Python |
|---------|-------------------|----------------|--------------|
| Non-intrusive | ✅ | ⚠️ | ⚠️ |
| Runtime generation | ✅ | ❌ | ❌ |
| Automatic discovery | ✅ | ❌ | ❌ |
| Modern C++ | ✅ | ✅ | ⚠️ |
| Type safety | ✅ | ✅ | ✅ |
| Documentation | ✅ | ⚠️ | ⚠️ |

## Requirements

- C++17 or later
- CMake 3.15+
- Python 3.7+
- pybind11 2.6+

## Troubleshooting

### "Class not registered" error

Make sure you call `register_classes()` before `bind_class()`:

```cpp
register_classes();  // Must come first!
gen.bind_class<MyClass>();
```

### Type conversion failures

Register custom converters for your types:

```cpp
gen.register_converter<MyType>(...);
```

### Linking errors

Ensure pybind11 is properly found:

```cmake
find_package(pybind11 CONFIG REQUIRED)
```

## Examples

See `py_example_usage.cpp` for complete examples including:
- Basic class binding
- Inheritance
- Free functions
- Custom converters
- STL containers

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions welcome! Please see CONTRIBUTING.md for guidelines.

## Resources

- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [Rosetta Documentation](https://rosetta.readthedocs.io/)
- [Python/C API](https://docs.python.org/3/c-api/)