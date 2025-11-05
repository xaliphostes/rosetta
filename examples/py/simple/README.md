# Rosetta Python Binding Generator

A **non-intrusive** Python binding generator using Rosetta introspection and pybind11. 

## Key Features

✨ **Zero Intrusion**: Your C++ classes don't need to inherit from anything  
🔍 **Pure Reflection**: Uses Rosetta metadata to generate bindings automatically  
🎯 **Type Safe**: Automatic type conversion between Python and C++  
🚀 **Easy to Use**: Bind entire classes with a single line of code  
📦 **Full Featured**: Supports constructors, fields, methods, properties, and validation

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Your C++ Classes                          │
│                  (No modifications needed)                   │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                 Rosetta Registration                         │
│              (ROSETTA_REGISTER_CLASS)                        │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│              Rosetta Metadata Registry                       │
│         (ClassMetadata, Fields, Methods, etc.)               │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│            PyBindingGenerator (pybind11)                     │
│    • Reads metadata from registry                            │
│    • Creates pybind11 class bindings                         │
│    • Generates property getters/setters                      │
│    • Wraps method calls with type conversion                 │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    Python Module                             │
│              (Native Python objects)                         │
└─────────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Define Your C++ Class (No Special Requirements!)

```cpp
class Vector3D {
public:
    double x, y, z;
    
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len; y /= len; z /= len;
        }
    }
};
```

### 2. Register with Rosetta

```cpp
void register_classes() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}
```

### 3. Create Python Module

```cpp
#include "pybind11_generator.h"

PYBIND11_MODULE(my_module, m) {
    register_classes();
    
    rosetta::bindings::create_bindings(m)
        .bind_class<Vector3D>()
        .add_utilities();
}
```

### 4. Use from Python!

```python
import my_module

# Create objects
v = my_module.Vector3D(3, 4, 12)

# Access properties
print(f"x={v.x}, y={v.y}, z={v.z}")  # x=3, y=4, z=12

# Call methods
print(f"length: {v.length()}")  # length: 13.0

# Modify properties
v.x = 6
v.y = 8

# Call methods that modify the object
v.normalize()
print(f"normalized: ({v.x}, {v.y}, {v.z})")
```

## Advanced Features

### Properties (Getter/Setter Pattern)

The generator supports C++ classes with getter/setter methods:

```cpp
class Person {
private:
    std::string name_;
    int age_;
    
public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& n) { name_ = n; }
    
    int getAge() const { return age_; }
    void setAge(int a) { age_ = a; }
};

// Register with properties
ROSETTA_REGISTER_CLASS(Person)
    .property<std::string>("name", &Person::getName, &Person::setName)
    .property<int>("age", &Person::getAge, &Person::setAge);
```

Python usage:
```python
p = Person("Alice", 30)
print(p.name)  # "Alice"
p.name = "Bob"  # Calls setName
p.age += 1      # Calls getAge then setAge
```

### Read-Only Properties

```cpp
class Circle {
private:
    double radius_;
public:
    double getRadius() const { return radius_; }
    void setRadius(double r) { radius_ = r; }
    
    // Computed property - read only
    double getArea() const { return M_PI * radius_ * radius_; }
};

ROSETTA_REGISTER_CLASS(Circle)
    .property<double>("radius", &Circle::getRadius, &Circle::setRadius)
    .readonly_property<double>("area", &Circle::getArea);
```

Python usage:
```python
c = Circle(5.0)
print(c.area)  # 78.53981633974483
c.radius = 10  # OK
c.area = 100   # AttributeError: read-only property
```

### Multiple Constructors

The generator automatically detects and binds all registered constructors:

```cpp
ROSETTA_REGISTER_CLASS(Vector3D)
    .constructor<>()                          // Default
    .constructor<double, double, double>();   // Parametric
```

Python usage:
```python
v1 = Vector3D()           # Default constructor
v2 = Vector3D(1, 2, 3)    # Parametric constructor
```

### Type Safety

The generator includes automatic type checking and conversion:

```python
v = Vector3D()
v.x = 5        # OK: int converts to double
v.x = 3.14     # OK: double
v.x = "oops"   # Error: cannot convert string to double
```

### Utility Functions

The generator adds helpful utilities to your module:

```python
# List all registered classes
classes = my_module.list_classes()
print(classes)  # ['Vector3D', 'Rectangle', 'Person', ...]

# Get class information
info = my_module.get_class_info('Vector3D')
print(info)
# {
#   'name': 'Vector3D',
#   'is_abstract': False,
#   'is_polymorphic': False,
#   'has_virtual_destructor': False,
#   'base_count': 0
# }

# Get Rosetta version
print(my_module.version())  # "1.0.0"
```

## Type Conversion

The generator automatically handles conversion between Python and C++ types:

| C++ Type       | Python Type  | Notes                          |
|----------------|--------------|--------------------------------|
| `int`          | `int`        | Direct conversion              |
| `double`       | `float`      | Automatic conversion           |
| `float`        | `float`      | Automatic conversion           |
| `bool`         | `bool`       | Direct conversion              |
| `std::string`  | `str`        | Automatic conversion           |
| `size_t`       | `int`        | Automatic conversion           |
| Custom classes | Object       | Wrapped with pybind11          |

## Building

### Requirements

- CMake 3.15+
- C++20 compiler
- Python 3.6+
- pybind11 (automatically fetched if not found)

### Build Steps

```bash
mkdir build
cd build
cmake ..
make
```

### Test

```bash
python test_bindings.py
```

## Comparison with Other Approaches

### Traditional pybind11 (Manual Binding)

```cpp
// Manual approach - repetitive and error-prone
PYBIND11_MODULE(example, m) {
    py::class_<Vector3D>(m, "Vector3D")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_readwrite("x", &Vector3D::x)
        .def_readwrite("y", &Vector3D::y)
        .def_readwrite("z", &Vector3D::z)
        .def("length", &Vector3D::length)
        .def("normalize", &Vector3D::normalize);
    
    py::class_<Rectangle>(m, "Rectangle")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def_readwrite("width", &Rectangle::width)
        .def_readwrite("height", &Rectangle::height)
        .def("area", &Rectangle::area)
        .def("perimeter", &Rectangle::perimeter);
    
    // ... repeat for every class ...
}
```

### Rosetta Approach (Automatic)

```cpp
// Rosetta approach - simple and automatic
PYBIND11_MODULE(example, m) {
    register_classes();  // Already defined elsewhere
    
    rosetta::bindings::create_bindings(m)
        .bind_class<Vector3D>()
        .bind_class<Rectangle>()
        .add_utilities();
}
```

### Key Advantages

✅ **DRY Principle**: Register once with Rosetta, use everywhere  
✅ **Maintainability**: Add a field/method once, available in all bindings  
✅ **Consistency**: Same metadata for Python, JavaScript, serialization, etc.  
✅ **Less Code**: ~90% reduction in binding boilerplate  
✅ **Type Safety**: Automatic type checking and conversion  

## Architecture Details

### Component Overview

1. **Type Converters**: Convert between `rosetta::core::Any` and `py::object`
2. **PyClassBinder**: Template class that binds a single C++ class
3. **PyBindingGenerator**: Main interface for binding multiple classes
4. **Utility Functions**: Helper functions for introspection

### Binding Process

```
Registration Phase:
  1. User registers class with Rosetta
  2. Metadata stored in Registry
  
Binding Phase:
  3. PyBindingGenerator reads metadata
  4. For each registered constructor:
     - Detect arity (number of parameters)
     - Create lambda wrapper
     - Register with pybind11
  5. For each field:
     - Create getter/setter lambdas
     - Register as property
  6. For each method:
     - Create wrapper with type conversion
     - Register with pybind11

Runtime Phase:
  7. Python calls method/property
  8. Wrapper converts Python args to Any
  9. Rosetta invokes C++ method
  10. Result converted back to Python
```

### Error Handling

The generator includes comprehensive error handling:

- **Type Mismatch**: Clear error messages when types don't match
- **Argument Count**: Validates number of arguments
- **Property Access**: Enforces read-only properties
- **Validation**: Preserves C++ exceptions and validation logic

## Examples

See the following files for complete examples:
- `pybind11_generator.h` - The generator implementation
- `binding_example.cxx` - Example C++ binding code
- `test_bindings.py` - Comprehensive test suite
- `CMakeLists.txt` - Build configuration

## Performance

The generator adds minimal overhead:
- **Constructor**: One virtual call to Rosetta + type conversions
- **Method Call**: One map lookup + type conversions
- **Property Access**: One map lookup + one type conversion

For most applications, this overhead is negligible compared to the Python/C++ boundary crossing.

## Future Enhancements

Potential improvements:
- [ ] Support for overloaded methods
- [ ] Container type conversion (std::vector ↔ list)
- [ ] Inheritance support
- [ ] Custom type converters
- [ ] Automatic docstring generation from metadata
- [ ] Numpy array integration
- [ ] Enum support

## License

This generator follows Rosetta's LGPL v3 license.

## See Also

- [Rosetta Core Documentation](../README.md)
- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [JavaScript Generator](../js/napi_generator.h)