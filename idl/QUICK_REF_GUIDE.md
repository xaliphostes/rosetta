# Rosetta IDL Quick Reference

## Command Line Usage

```bash
# Validate an IDL file
python rosetta_idl_parser.py interface.yaml --validate-only

# Generate Rosetta registration code
python rosetta_idl_parser.py interface.yaml -o output/

# Generate Python bindings only
python py_binding_generator.py interface.yaml output.cpp

# Generate both Rosetta and Python bindings
python rosetta_idl_parser.py interface.yaml -o output/ --python

# Verbose output
python rosetta_idl_parser.py interface.yaml -o output/ -v

# Run tests
python test_idl_parser.py

# Run examples
python usage_examples.py

# See CLI examples
python cli_examples.py
```

## Python API

### Parse IDL

```python
from rosetta_idl_parser import parse_idl

idl = parse_idl("interface.yaml")
print(f"Module: {idl.module.name}")
```

### Generate Bindings

```python
from py_binding_generator import generate_python_bindings

generate_python_bindings("interface.yaml", "binding.cpp")
```

## IDL Syntax Cheat Sheet

### Module

```yaml
module:
  name: my_module
  version: "1.0.0"
  namespace: my::ns # from the User library
  description: "Description"
```

### Includes

```yaml
includes:
  - <vector>           # System
  - mylib/header.h     # User
```

### Type Converters

```yaml
converters:
  - type: "std::vector<MyType>"
  - type: "std::map<K, V>"
  - type: "std::set<T>"
  - type: "std::array<T, N>"
```

### Basic Class

```yaml
classes:
  - name: MyClass
    constructors:
      - parameters: []
      - parameters:
          - name: value
            type: int
    
    fields:
      - name: field
        type: int
        access: rw  # rw, ro, wo
    
    methods:
      - name: method
        returns: int
        const: true
        parameters:
          - name: arg
            type: int
```

### Properties

```yaml
fields:
  - name: prop
    type: int
    access: rw
    getter: getProp
    setter: setProp
```

### Inheritance

```yaml
classes:
  - name: Base
    is_abstract: true
    methods:
      - name: virtual_func
        returns: void
        pure_virtual: true
  
  - name: Derived
    base_classes:
      - name: Base
        inheritance: normal  # normal or virtual
        access: public
    methods:
      - name: virtual_func
        returns: void
        override: true
```

### Free Functions

```yaml
functions:
  - name: my_function
    returns: int
    parameters:
      - name: arg
        type: int
        default: "0"
```

### Utilities

```yaml
utilities:
  version_info: true
  list_classes: true
  type_inspection: true
```

## Field Access Modes

- `rw` - Read-Write (default)
- `ro` - Read-Only
- `wo` - Write-Only

## Method Modifiers

- `const: true` - Const method
- `virtual: true` - Virtual method
- `pure_virtual: true` - Pure virtual method
- `override: true` - Override method
- `static: true` - Static method

## Inheritance Types

- `normal` - Normal inheritance
- `virtual` - Virtual inheritance

## Access Specifiers

- `public` (default)
- `protected`
- `private`

## Common Patterns

### Value Type

```yaml
classes:
  - name: Vec3
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
      - {name: x, type: double, access: rw}
      - {name: y, type: double, access: rw}
      - {name: z, type: double, access: rw}
    methods:
      - name: length
        returns: double
        const: true
```

### Abstract Interface

```yaml
classes:
  - name: Interface
    is_abstract: true
    methods:
      - name: method1
        returns: void
        pure_virtual: true
      - name: method2
        returns: int
        const: true
        pure_virtual: true
```

### Container Class

```yaml
converters:
  - type: "std::vector<Item>"

classes:
  - name: Container
    methods:
      - name: getItems
        returns: "std::vector<Item>"
        const: true
      - name: setItems
        parameters:
          - name: items
            type: "const std::vector<Item>&"
```

### Property-based Class

```yaml
classes:
  - name: Entity
    auto_detect_properties: true
    methods:
      - name: getName
        returns: std::string
        const: true
      - name: setName
        parameters:
          - name: name
            type: "const std::string&"
```

## Error Messages

### Duplicate Class Name
```
Error: Duplicate class name: MyClass
```
**Fix**: Rename one of the classes

### Circular Inheritance
```
Error: Circular inheritance detected for class: ClassA
```
**Fix**: Remove the circular dependency

### Missing Base Class
```
Warning: Base class 'Base' for 'Derived' not found in IDL
```
**Fix**: Define the base class or verify the name

## Tips

1. **Use 2-space indentation** for YAML files
2. **Quote template types** with angle brackets: `"std::vector<int>"`
3. **Register converters** before binding classes that use them
4. **Validate** your IDL before code generation
5. **Use descriptions** to document your interfaces
6. **Version your modules** with semantic versioning
7. **Group related classes** in the same IDL file
8. **Use auto_detect_properties** for classes with many get/set pairs

## Validation Checklist

- [ ] No duplicate class names
- [ ] No duplicate function names  
- [ ] No circular inheritance
- [ ] Base classes are defined
- [ ] All types are valid C++ types
- [ ] Container types have converters
- [ ] Access modes are valid (rw/ro/wo)
- [ ] Inheritance types are valid (normal/virtual)

## Generated Code Structure

```cpp
// 1. Includes
#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>
// ... user includes

// 2. Rosetta registrations
void register_MODULE_classes() {
    ROSETTA_REGISTER_CLASS(MyClass)
        .constructor<>()
        .field("field", &MyClass::field)
        .method("method", &MyClass::method);
}

// 3. Python module
BEGIN_PY_MODULE(MODULE, "Description") {
    register_MODULE_classes();
    
    // Type converters
    rosetta::py::bind_vector_type<MyType>();
    
    // Bind classes
    BIND_PY_CLASS(MyClass);
    
    // Bind functions
    BIND_FUNCTION(my_function, "");
    
    // Utilities
    BIND_PY_UTILITIES();
}
END_PY_MODULE()
```

## Common Use Cases

### Game Engine

```yaml
classes:
  - name: GameObject
  - name: Component
  - name: Transform
  - name: Scene
```

### Graphics Library

```yaml
classes:
  - name: Vector2D/3D
  - name: Matrix4x4
  - name: Color
  - name: Mesh
```

### Physics Engine

```yaml
classes:
  - name: RigidBody
  - name: Collider
  - name: Force
  - name: PhysicsWorld
```

### Data Processing

```yaml
classes:
  - name: Dataset
  - name: DataPoint
  - name: Processor
  - name: Result
```

## Resources

- Full documentation: README.md
- Examples: usage_examples.py
- Test suite: test_idl_parser.py
- Parser code: rosetta_idl_parser.py
- Generator code: py_binding_generator.py