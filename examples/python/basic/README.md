# Automatic Python Bindings for C++ Introspection

This directory contains an automatic binding generator that creates Python modules from C++ classes using the introspection system. It requires minimal code to expose entire C++ classes to Python with full type safety and natural Pythonic syntax.

## Features

- **Automatic Property Binding**: All class members become Python properties with natural get/set syntax
- **Method Binding**: All class methods become Python functions with automatic parameter conversion
- **Type Safety**: Automatic type conversion between Python and C++ types
- **Introspection Utilities**: Access to reflection information from Python
- **Pythonic Naming**: Automatic conversion of camelCase to snake_case (optional)
- **Error Handling**: Proper Python exceptions for invalid operations
- **Factory Functions**: Automatic creation of object factory methods

## Building

### Prerequisites

- Python 3.7 or later
- C++20 compatible compiler
- CMake 3.15 or later
- pybind11 (automatically fetched via CMake)

### Build with CMake

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# The Python module will be in build/pyintro.so (or .pyd on Windows)
```

### Build with setuptools (Alternative)

```bash
# Install in development mode
pip install -e .

# Or build wheel
pip install build
python -m build
```

### Using the Module

```bash
# Add build directory to Python path
export PYTHONPATH=$PYTHONPATH:/path/to/build

# Or install the module
pip install .
```

## Running the example

- make sure the python version is >=3.13
- Stay in the build folder
```bash
python3 ../test.py
```

## Quick Start

### 1. Define Your C++ Classes

```cpp
#include <rosetta/introspectable.h>

class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)    
public:
    Person();
    Person(const std::string& n, int a, double h);

    // Getters and setters
    std::string getName() const;
    void setName(const std::string& n);
    int getAge() const;
    void setAge(int a);
    
    // Methods
    void introduce() const;
    void celebrateBirthday();
    std::string getDescription() const;

private:
    std::string name;
    int age;
    double height;
    bool isActive;
};

void Person::registerIntrospection(rosetta::TypeRegistrar<Person> reg) {
    reg.member("name", &Person::name)
       .member("age", &Person::age)
       .member("height", &Person::height)
       .member("isActive", &Person::isActive)
       .method("getName", &Person::getName)
       .method("setName", &Person::setName)
       .method("getAge", &Person::getAge)
       .method("setAge", &Person::setAge)
       .method("introduce", &Person::introduce)
       .method("celebrateBirthday", &Person::celebrateBirthday)
       .method("getDescription", &Person::getDescription);
}
```

### 2. Create Python Module (Only 3 Lines!)

```cpp
#include <rosetta/generators/py.h>

PYBIND11_MODULE(introspection_demo, m) {
    m.doc() = "Automatic Python bindings using C++ introspection";
    
    rosetta::PyGenerator generator(m);
    generator.bind_classes<Person, Vehicle>();  // Bind multiple classes at once!
}
```

### 3. Use from Python

```python
import introspection_demo

# Create objects using constructors
person = introspection_demo.Person("Alice", 30, 1.65)
vehicle = introspection_demo.Vehicle("Honda", "Civic", 2022)

# Use auto-generated properties (direct member access)
print(f"Person name: {person.name}")
print(f"Person age: {person.age}")
print(f"Vehicle brand: {vehicle.brand}")

# Modify properties
person.name = "Bob"
person.age = 25
vehicle.mileage = 15000.5

# Call methods
person.introduce()
person.celebrate_birthday()  # Note: pybind11 converts camelCase to snake_case

vehicle.start()
vehicle.drive(100.5)
print(vehicle.get_info())

# Use introspection utilities
print("Class name:", person.get_class_name())
print("Members:", person.get_member_names())
print("Methods:", person.get_method_names())

# Dynamic access
print("Age via introspection:", person.get_member_value("age"))
person.set_member_value("height", 1.80)
person.call_method("introduce", [])

# JSON export
print("Person as JSON:", person.to_json())

# Module utilities
print("Available classes:", introspection_demo.get_all_classes())
default_person = introspection_demo.create_person()
default_vehicle = introspection_demo.create_vehicle()
```

## API Reference

### Automatic Bindings

The generator automatically creates:

#### **Properties**: Direct access to C++ members
```python
obj.member_name = value    # Set
print(obj.member_name)     # Get
```

#### **Methods**: Direct calls to C++ methods
```python
obj.method_name(arg1, arg2)
result = obj.get_value()
```

#### **Constructors**: Automatic binding of default constructor
```python
person = Person()                    # Default constructor
person = Person("Alice", 30, 1.65)   # Parameterized (if available)
```

### Introspection API

Every bound object includes these methods:

- `get_class_name()` → `str` - Get class name
- `get_member_names()` → `List[str]` - Get all member names  
- `get_method_names()` → `List[str]` - Get all method names
- `has_member(name)` → `bool` - Check if member exists
- `has_method(name)` → `bool` - Check if method exists
- `get_member_value(name)` → `Any` - Get member value by name
- `set_member_value(name, value)` → `None` - Set member value by name
- `call_method(name, args)` → `Any` - Call method by name
- `to_json()` → `str` - Export object to JSON
- `print_class_info()` → `None` - Print complete class information
- `print_member_value(name)` → `None` - Print member with type info

### Module Utilities

The module automatically provides:

```python
# Get all bound classes
classes = introspection_demo.get_all_classes()

# Factory functions for each class
person = introspection_demo.create_person()
vehicle = introspection_demo.create_vehicle()
```

## Supported Types

### C++ to Python
- `std::string` ↔ `str`
- `int` ↔ `int`
- `double`, `float` ↔ `float`
- `bool` ↔ `bool`
- `void` ↔ `None`
- `std::vector<T>` ↔ `List[T]`
- `std::map<K,V>` ↔ `Dict[K,V]`

### Type Conversion

The generator handles automatic type conversion:

```python
# Python → C++
person.age = 30              # int → int
person.name = "Alice"        # str → std::string
person.is_active = True      # bool → bool
person.height = 1.65         # float → double

# C++ → Python  
age = person.age             # int → int
name = person.name           # std::string → str
active = person.is_active    # bool → bool
```

## Error Handling

```python
try:
    person.nonexistent_member = "value"
except AttributeError as e:
    print(f"Error: {e}")  # "Member 'nonexistent_member' not found"

try:
    person.method_with_wrong_args()
except RuntimeError as e:
    print(f"Error: {e}")  # "Method 'method' expects 2 arguments, got 0"

try:
    person.set_member_value("age", "not a number")
except TypeError as e:
    print(f"Error: {e}")  # "Failed to convert Python object to 'int'"
```

## Advanced Features

### Method Chaining

If your C++ methods return `*this`, they support chaining in Python:

```python
person.set_name("Alice").set_age(30).set_height(1.65)
```

### Dynamic Introspection

```python
# Get all members and their current values
for member in person.get_member_names():
    value = person.get_member_value(member)
    print(f"{member} = {value}")

# Get all methods and their signatures
for method in person.get_method_names():
    print(f"Method: {method}")
```

### Batch Updates

```python
# Update multiple properties from dict
person_data = {
    "name": "Diana",
    "age": 28,
    "height": 1.68,
    "is_active": False
}

for key, value in person_data.items():
    if person.has_member(key):
        person.set_member_value(key, value)
```

### JSON Serialization

```python
# Export to JSON
json_str = person.to_json()
print(json_str)
# Output:
# {
#   "className": "Person",
#   "members": [
#     {"name": "name", "type": "string", "value": "Alice"},
#     {"name": "age", "type": "int", "value": 30},
#     ...
#   ]
# }

# Can be parsed with standard json module
import json
data = json.loads(json_str)
```

## Performance

The binding generator is optimized for:
- **Fast Property Access**: Direct member access through pybind11
- **Efficient Method Calls**: Minimal overhead for method invocation
- **Type Conversion**: Optimized conversion between Python and C++ types
- **Memory Management**: Automatic cleanup through smart pointers

### Benchmarks

```python
import time

# Property access performance
start = time.time()
for i in range(100000):
    person.age = i
    _ = person.age
end = time.time()
print(f"100k property operations: {(end-start)*1000:.2f}ms")
```

## Comparison with Manual Bindings

### Manual pybind11 Code (100+ lines)

```cpp
PYBIND11_MODULE(manual_demo, m) {
    py::class_<Person>(m, "Person")
        .def(py::init<>())
        .def(py::init<const std::string&, int, double>())
        .def_property("name", &Person::getName, &Person::setName)
        .def_property("age", &Person::getAge, &Person::setAge)
        .def_property("height", &Person::getHeight, &Person::setHeight)
        .def_property("is_active", &Person::getIsActive, &Person::setIsActive)
        .def("introduce", &Person::introduce)
        .def("celebrate_birthday", &Person::celebrateBirthday)
        .def("get_description", &Person::getDescription);
    
    // Repeat for every class...
}
```

### Automatic Bindings (3 lines)

```cpp
PYBIND11_MODULE(introspection_demo, m) {
    rosetta::PyGenerator generator(m);
    generator.bind_classes<Person, Vehicle>();
}
```

## Extending the Generator

### Custom Type Converters

To add support for custom types:

```cpp
// In PythonBindingGenerator:
// Register custom type at runtime
generator.register_type_converter(
    "Vector3D",
    // C++ to JS
    [](Napi::Env env, const std::any &value) -> Napi::Value {
        auto vec = std::any_cast<Vector3D>(value);
        auto obj = Napi::Object::New(env);
        obj.Set("x", vec.x);
        obj.Set("y", vec.y);
        obj.Set("z", vec.z);
        return obj;
    },
    // JS to C++
    [](const Napi::Value &js_val) -> std::any {
        auto obj = js_val.As<Napi::Object>();
        return Vector3D(obj.Get("x").As<Napi::Number>().FloatValue(),
                        obj.Get("y").As<Napi::Number>().FloatValue(),
                        obj.Get("z").As<Napi::Number>().FloatValue());
    });
```

### Custom Property Names

Override property names if needed:

```cpp
generator.bind_class<Person>()
    .rename_property("isActive", "active")  // is_active → active
    .rename_method("celebrateBirthday", "birthday");  // celebrate_birthday → birthday
```

## Troubleshooting

### Import Errors

```bash
# Check if module was built
ls build/*.so  # Linux/macOS
ls build/*.pyd  # Windows

# Check Python path
python -c "import sys; print(sys.path)"

# Add build directory to path
export PYTHONPATH=$PYTHONPATH:$(pwd)/build
```

### Compilation Errors

```bash
# Make sure pybind11 is installed
pip install pybind11

# Check C++ compiler
g++ --version  # Should support C++20

# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
make
```

### Type Conversion Errors

```python
# Check available types
print(person.get_member_names())

# Check type of a member
import introspection_demo
help(introspection_demo.Person.age)  # Shows type information
```

### Missing Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install python3-dev python3-pip cmake g++

# macOS
brew install python cmake

# Install Python packages
pip install pybind11 numpy
```

## Examples

See `test.py` for comprehensive usage examples including:
- Basic property and method access
- Constructor usage
- Error handling
- Dynamic introspection
- Performance testing
- Batch operations
- JSON serialization

## Integration with Other Python Tools

### NumPy Integration

```python
import numpy as np

# Create array of objects
persons = np.array([Person("Alice", 30, 1.65) for _ in range(10)])

# Vectorized operations (if methods support it)
for p in persons:
    p.celebrate_birthday()
```

### Pandas Integration

```python
import pandas as pd

# Create DataFrame from objects
persons = [Person("Alice", 30, 1.65), Person("Bob", 25, 1.80)]
df = pd.DataFrame([
    {
        'name': p.name,
        'age': p.age,
        'height': p.height,
        'active': p.is_active
    }
    for p in persons
])
```

### Type Hints

Add type hints for better IDE support:

```python
from typing import List, Optional
import introspection_demo as demo

def process_person(person: demo.Person) -> str:
    """Process a person and return description."""
    return person.get_description()

def create_people(count: int) -> List[demo.Person]:
    """Create multiple person objects."""
    return [demo.Person() for _ in range(count)]
```

## License

LGPL License - see LICENSE file for details

## Contributing

Contributions are welcome! Please:
1. Add tests for new features
2. Update documentation
3. Follow existing code style
4. Ensure all tests pass

## Support

For issues, questions, or contributions:
- GitHub Issues: [link to repo]
- Documentation: [link to docs]
- Examples: See `examples/` directory