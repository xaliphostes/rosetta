# Rosetta IDL Parser

A comprehensive Interface Description Language (IDL) parser for the Rosetta C++ introspection framework. This tool allows you to describe C++ classes, functions, and types in YAML format and automatically generate binding code for Python, JavaScript, and other scripting languages.

## Features

✨ **Complete Feature Set**:
- Parse YAML interface description files
- Support for classes, methods, fields, constructors
- Inheritance (normal and virtual)
- Abstract classes and pure virtual methods
- Properties with getter/setter methods
- Free functions
- Type converters for STL containers
- Automatic validation
- Code generation for Python bindings

🎯 **Supported C++ Features**:
- Fields (public, private, protected)
- Methods (const, virtual, pure virtual, override, static)
- Constructors
- Single and multiple inheritance
- Abstract base classes
- Properties with custom getters/setters
- STL containers (vector, map, set, array, etc.)

## Installation

```bash
# Install dependencies
pip install pyyaml

# Copy the parser files
cp rosetta_idl_parser.py /your/project/
cp py_binding_generator.py /your/project/
```

## Quick Start

### Command Line Interface

The parser can be used directly from the command line:

```bash
# Validate an IDL file
python rosetta_idl_parser.py interface.yaml --validate-only

# Generate Rosetta registration code
python rosetta_idl_parser.py interface.yaml -o output/

# Generate both Rosetta registration and Python bindings
python rosetta_idl_parser.py interface.yaml -o output/ --python

# Verbose output
python rosetta_idl_parser.py interface.yaml -o output/ -v

# See all options
python rosetta_idl_parser.py --help
```

**Command-Line Options:**
- `idl_file` - Path to the YAML IDL file (required)
- `-o, --output-dir` - Output directory for generated files (default: current directory)
- `--python` - Also generate Python bindings
- `-v, --verbose` - Verbose output with detailed information
- `--validate-only` - Only validate the IDL, do not generate code

### 1. Create an IDL File

Create a YAML file describing your C++ interface (e.g., `geometry.yaml`):

```yaml
module:
  name: geometry
  version: "1.0.0"
  namespace: geo

includes:
  - <vector>
  - <string>
  - mylib/Vector3D.h

converters:
  - type: "std::vector<Vector3D>"

classes:
  - name: Vector3D
    description: "3D vector representation"
    
    constructors:
      - parameters: []
      - parameters:
          - name: x
            type: double
          - name: y
            type: double
          - name: z
            type: double
    
    fields:
      - name: x
        type: double
        access: rw
      - name: y
        type: double
        access: rw
      - name: z
        type: double
        access: rw
    
    methods:
      - name: length
        returns: double
        const: true

functions:
  - name: create_vector
    returns: Vector3D
    parameters:
      - name: x
        type: double
      - name: y
        type: double
      - name: z
        type: double

utilities:
  version_info: true
  list_classes: true
```

### 2. Generate Bindings

Using the command line:

```bash
# Generate Rosetta registration
python rosetta_idl_parser.py geometry.yaml -o output/

# Generate both Rosetta and Python bindings
python rosetta_idl_parser.py geometry.yaml -o output/ --python
```

Or using Python API:

```python
from rosetta_idl_parser import parse_idl, write_rosetta_bindings
from pathlib import Path

# Parse the IDL file
idl = parse_idl("geometry.yaml")

# Generate Rosetta registration
write_rosetta_bindings(idl, Path("output/"))

# Generate Python bindings
from py_binding_generator import generate_python_bindings
generate_python_bindings("geometry.yaml", "output/geometry_binding.cpp")
```

### 3. Compile and Use

## IDL File Format

### Module Configuration

```yaml
module:
  name: my_module          # Required: module name
  version: "1.0.0"         # Optional: version string
  namespace: my::ns        # Optional: C++ namespace
  description: "..."       # Optional: module description
```

### Includes

```yaml
includes:
  - <vector>               # System include: #include <vector>
  - <string>               # System include: #include <string>
  - mylib/header.h         # User include: #include "mylib/header.h"
  - path: other.h          # Explicit format
    system: false
```

### Type Converters

Register automatic type converters for STL containers:

```yaml
converters:
  - type: "std::vector<MyClass>"
  - type: "std::map<std::string, MyClass>"
  - type: "std::set<int>"
  - type: "std::array<double, 16>"
  - type: "std::unordered_map<int, std::string>"
  - type: "std::deque<MyClass>"
```

### Classes

#### Basic Class

```yaml
classes:
  - name: MyClass
    cpp_class: MyClass     # Optional: if C++ name differs
    description: "..."     # Optional: class description
    
    constructors:
      - description: "Default constructor"
        parameters: []
      
      - description: "Parametric constructor"
        parameters:
          - name: value
            type: int
            default: "0"   # Optional: default value
    
    fields:
      - name: my_field
        type: int
        access: rw         # rw (read-write), ro (read-only), wo (write-only)
        description: "..." # Optional: field description
    
    methods:
      - name: myMethod
        returns: int
        const: true        # Optional: const method
        virtual: false     # Optional: virtual method
        static: false      # Optional: static method
        parameters:
          - name: arg
            type: int
```

#### Properties with Getters/Setters

```yaml
fields:
  - name: value
    type: int
    access: rw
    getter: getValue      # Method name for getter
    setter: setValue      # Method name for setter
  
  - name: read_only
    type: double
    access: ro
    getter: getReadOnly
  
  - name: write_only
    type: std::string
    access: wo
    setter: setWriteOnly
```

#### Auto-Detect Properties

Automatically detect properties from get/set method pairs:

```yaml
classes:
  - name: MyClass
    auto_detect_properties: true
    
    methods:
      - name: getValue
        returns: int
        const: true
      
      - name: setValue
        parameters:
          - name: value
            type: int
```

This will automatically create a "value" property.

#### Inheritance

```yaml
classes:
  - name: Base
    is_abstract: true
    methods:
      - name: virtual_method
        returns: void
        pure_virtual: true
  
  - name: Derived
    base_classes:
      - name: Base
        inheritance: normal  # normal or virtual
        access: public       # public, protected, or private
    
    methods:
      - name: virtual_method
        returns: void
        override: true
```

### Free Functions

```yaml
functions:
  - name: my_function
    cpp_name: my_function  # Optional: if C++ name differs
    returns: int
    parameters:
      - name: arg1
        type: int
        default: "0"       # Optional: default value
      - name: arg2
        type: double
    description: "..."     # Optional: function description
```

### Utilities

```yaml
utilities:
  version_info: true       # Add version() function
  list_classes: true       # Add list_classes() function
  type_inspection: true    # Add type inspection utilities
  get_class_info: true     # Add get_class_info() function
```

## API Reference

### IDLParser

```python
from rosetta_idl_parser import IDLParser

parser = IDLParser()

# Parse from file
idl = parser.parse_file("interface.yaml")

# Parse from string
yaml_content = "..."
idl = parser.parse_string(yaml_content)

# Parse from dictionary
data = {...}
idl = parser.parse_dict(data)

# Validate
if parser.validate(idl):
    print("Valid IDL")

# Check for errors
if parser.has_errors():
    parser.print_diagnostics()
```

### InterfaceDefinition

```python
# Access module information
print(idl.module.name)
print(idl.module.version)
print(idl.module.namespace)

# Access classes
for cls in idl.classes:
    print(f"Class: {cls.name}")
    print(f"  Fields: {[f.name for f in cls.fields]}")
    print(f"  Methods: {[m.name for m in cls.methods]}")
    print(f"  Constructors: {len(cls.constructors)}")

# Get specific class
vector_class = idl.get_class("Vector3D")

# Access functions
for func in idl.functions:
    print(f"Function: {func.name}")
    print(f"  Returns: {func.returns}")
    print(f"  Parameters: {len(func.parameters)}")

# Access converters
for conv in idl.converters:
    print(f"Converter: {conv.type}")
```

### PythonBindingGenerator

```python
from py_binding_generator import PythonBindingGenerator

generator = PythonBindingGenerator(idl)
generator.generate(output_path)
```

## Examples

### Example 1: Simple Value Type

```yaml
module:
  name: math
  version: "1.0.0"

includes:
  - <cmath>

classes:
  - name: Vec2
    constructors:
      - parameters: []
      - parameters:
          - name: x
            type: double
          - name: y
            type: double
    
    fields:
      - name: x
        type: double
        access: rw
      - name: y
        type: double
        access: rw
    
    methods:
      - name: length
        returns: double
        const: true
      
      - name: normalize
        returns: Vec2
        const: true
```

### Example 2: Abstract Base Class with Inheritance

```yaml
module:
  name: shapes
  version: "1.0.0"

classes:
  - name: Shape
    is_abstract: true
    is_polymorphic: true
    
    methods:
      - name: area
        returns: double
        const: true
        pure_virtual: true
      
      - name: perimeter
        returns: double
        const: true
        pure_virtual: true
  
  - name: Rectangle
    base_classes:
      - name: Shape
    
    constructors:
      - parameters:
          - name: width
            type: double
          - name: height
            type: double
    
    methods:
      - name: area
        returns: double
        const: true
        override: true
      
      - name: perimeter
        returns: double
        const: true
        override: true
```

### Example 3: Container Types

```yaml
module:
  name: scene
  version: "1.0.0"

includes:
  - <vector>
  - <map>
  - mylib/Object.h

converters:
  - type: "std::vector<Object>"
  - type: "std::map<std::string, Object>"

classes:
  - name: Scene
    methods:
      - name: getObjects
        returns: "std::vector<Object>"
        const: true
      
      - name: setObjects
        parameters:
          - name: objects
            type: "const std::vector<Object>&"
      
      - name: findObject
        returns: Object
        const: true
        parameters:
          - name: name
            type: "const std::string&"
```

## Advanced Features

### Virtual Inheritance

```yaml
classes:
  - name: Derived
    base_classes:
      - name: Base
        inheritance: virtual
        access: public
```

### Static Methods

```yaml
methods:
  - name: create
    returns: MyClass
    static: true
    parameters:
      - name: value
        type: int
```

### Custom Type Converters

```yaml
converters:
  - type: "std::function<void(int)>"
    custom_converter: "my_custom_converter"
```

## Validation

The parser includes built-in validation:

- ✅ Duplicate class names
- ✅ Duplicate function names
- ✅ Circular inheritance
- ✅ Missing base classes
- ✅ Invalid access specifiers
- ✅ Invalid inheritance types

## Testing

Run the test suite:

```bash
python test_idl_parser.py
```

Tests include:
- Basic parsing
- Comprehensive features
- Inheritance
- Properties
- Type converters
- Validation
- Code generation
- Access modes
- Includes

## Generated Code

The generator produces C++ code that integrates with the Rosetta framework:

```cpp
// Auto-generated binding
#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>

void register_geometry_classes() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length);
}

BEGIN_PY_MODULE(geometry, "Geometry module") {
    register_geometry_classes();
    
    rosetta::py::bind_vector_type<Vector3D>();
    
    BIND_PY_CLASS(Vector3D);
    BIND_FUNCTION(create_vector, "Create a vector");
    BIND_PY_UTILITIES();
}
END_PY_MODULE()
```

## Best Practices

1. **Use Descriptive Names**: Give clear names to classes, methods, and parameters
2. **Add Descriptions**: Document your interfaces with description fields
3. **Group Related Classes**: Keep related classes in the same IDL file
4. **Register Converters**: Always register converters for container types
5. **Validate Before Generating**: Always validate your IDL before code generation
6. **Version Your Modules**: Use semantic versioning for your modules

## Troubleshooting

### Parse Errors

```python
parser = IDLParser()
idl = parser.parse_file("interface.yaml")

if parser.has_errors():
    print("Errors found:")
    for error in parser.get_errors():
        print(f"  - {error}")
```

### Validation Errors

```python
if not parser.validate(idl):
    parser.print_diagnostics()
```

### Common Issues

- **YAML Syntax**: Ensure proper indentation (use spaces, not tabs)
- **Type Names**: Use exact C++ type names including namespaces
- **Container Types**: Always register converters for container types
- **Inheritance**: Ensure base classes are defined before derived classes

## Contributing

Contributions are welcome! Please:

1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Ensure all tests pass

## License

LGPL 3

## Authors

xaliphostes

## Changelog

### Version 1.0.0 (2024)
- Initial release
- Full YAML parsing support
- Python binding generation
- Comprehensive validation
- Complete test suite