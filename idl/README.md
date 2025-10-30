# Rosetta Interface Description Language (IDL)

A declarative system for generating language bindings from C++ code using Rosetta introspection.

## Overview

Instead of manually writing binding code for each target language (JavaScript, Python, C#, etc.), the IDL system lets you describe your interface once in a simple YAML or JSON file, then automatically generates all necessary binding code.

## Features

- **Declarative**: Describe interfaces in YAML/JSON instead of writing C++ binding code
- **Multi-language**: Generate bindings for multiple languages from a single description
- **Type-safe**: Leverages Rosetta's introspection system for type safety
- **Maintainable**: Changes to the interface only require updating the IDL file
- **Extensible**: Easy to add support for new target languages

## Quick Start

### 1. Write an IDL File

Create `myproject.idl.yaml`:

```yaml
module:
  name: geometry
  version: "1.0.0"
  namespace: myproject

includes:
  - vector
  - optional
  - myproject/Vector3D.h

converters:
  - type: "std::vector<int>"
    kind: vector
  - type: "std::vector<double>"
    kind: vector
  - type: "std::optional<double>"
    kind: optional

classes:
  - name: Vector3D
    description: "3D vector representation"
    
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
        description: "Calculate vector length"
      
      - name: normalize
        returns: void
        description: "Normalize the vector"

utilities:
  - version_info: true
  - list_classes: true
  - type_inspection: true
```

### 2. Generate Bindings

```bash
# Generate JavaScript bindings
python rosetta_gen.py --input myproject.idl.yaml --lang js --output ./bindings

# Generate Python bindings (coming soon)
python rosetta_gen.py --input myproject.idl.yaml --lang python --output ./bindings

# Generate all supported languages
python rosetta_gen.py --input myproject.idl.yaml --lang all --output ./bindings
```

### 3. Build and Test

```bash
cd bindings/javascript
npm install
node test.js
```

## IDL File Format

### Module Configuration

```yaml
module:
  name: module_name          # Required: module identifier
  version: "1.0.0"           # Optional: semantic version
  namespace: cpp_namespace   # Optional: C++ namespace
  description: "..."         # Optional: module description
```

### Include Files

Specify C++ header files to include in the generated binding code. The system automatically formats includes correctly:

```yaml
includes:
  # System includes (STL, standard libraries) - use simple names
  - vector                   # Becomes: #include <vector>
  - string                   # Becomes: #include <string>
  - memory                   # Becomes: #include <memory>
  - optional                 # Becomes: #include <optional>
  
  # User includes - use path format (with / or .h extension)
  - mylib/Vector3D.h        # Becomes: #include "mylib/Vector3D.h"
  - Vector3D.h              # Becomes: #include "Vector3D.h"
  - myproject/core/types.hpp # Becomes: #include "myproject/core/types.hpp"
```

**Rules:**
- Names without `/` or `.h`/`.hpp` → angle brackets `<>`
- Names with `/` or `.h`/`.hpp` → quotes `""`

### Type Converters

Register converters for STL types:

```yaml
converters:
  - type: "std::vector<int>"
    kind: vector
  
  - type: "std::vector<double>"
    kind: vector
  
  - type: "std::optional<double>"
    kind: optional
  
  - type: "std::map<std::string, int>"
    kind: map
  
  - type: "std::array<double, 3>"
    kind: array
```

Supported kinds:
- `vector`: `std::vector<T>`
- `optional`: `std::optional<T>`
- `map`: `std::map<K, V>`
- `array`: `std::array<T, N>`
- `custom`: Custom converter (requires manual registration)

### Classes

```yaml
classes:
  - name: ClassName              # Required: class name in target language
    cpp_class: CppClassName      # Optional: C++ class name if different
    description: "..."           # Optional: class description
    base_classes:                # Optional: inheritance
      - BaseClass1
      - BaseClass2
    
    fields:
      - name: field_name         # Required: field name
        type: field_type         # Required: C++ type
        access: rw               # Optional: rw (default), ro, wo
        description: "..."       # Optional: field description
        cpp_name: cpp_field      # Optional: C++ field name if different
    
    methods:
      - name: method_name        # Required: method name
        returns: return_type     # Optional: return type (default: void)
        const: true              # Optional: const method (default: false)
        description: "..."       # Optional: method description
        cpp_name: cpp_method     # Optional: C++ method name if different
        parameters:              # Optional: method parameters
          - name: param_name
            type: param_type
            default: "value"     # Optional: default value
```

### Free Functions

```yaml
functions:
  - name: function_name        # Required: function name
    cpp_name: cpp_func         # Optional: C++ function name if different
    returns: return_type       # Optional: return type (default: void)
    description: "..."         # Optional: function description
    parameters:
      - name: param_name
        type: param_type
        default: "value"       # Optional: default value
```

### Utilities

```yaml
utilities:
  version_info: true           # Include version information
  list_classes: true           # Include class listing function
  type_inspection: true        # Include type inspection utilities
```

## Field Access Modes

- `rw` (read-write): Default, generates getter and setter
- `ro` (read-only): Only generates getter
- `wo` (write-only): Only generates setter

## Examples

### Simple Class

```yaml
classes:
  - name: Point2D
    fields:
      - name: x
        type: double
      - name: y
        type: double
    methods:
      - name: distance
        returns: double
        const: true
        parameters:
          - name: other
            type: Point2D
```

### Class with Complex Types

```yaml
classes:
  - name: DataContainer
    fields:
      - name: name
        type: std::string
      
      - name: values
        type: "std::vector<int>"
      
      - name: threshold
        type: "std::optional<double>"
      
      - name: metadata
        type: "std::map<std::string, std::string>"
```

### Class with Inheritance

```yaml
classes:
  - name: Shape
    methods:
      - name: area
        returns: double
        const: true
  
  - name: Circle
    base_classes:
      - Shape
    fields:
      - name: radius
        type: double
    methods:
      - name: area
        returns: double
        const: true
```

### Read-Only Fields

```yaml
classes:
  - name: Config
    fields:
      - name: version
        type: std::string
        access: ro          # Read-only
      
      - name: debug_mode
        type: bool
        access: rw          # Read-write (default)
```

## Generated Files

### JavaScript Bindings

When generating JavaScript bindings, the following files are created:

- `binding.cxx`: N-API binding code
- `binding.gyp`: Build configuration
- `package.json`: NPM package configuration
- `test.js`: Basic test file

### Python Bindings (Coming Soon)

- `bindings.cpp`: PyBind11 binding code
- `setup.py`: Build configuration
- `test.py`: Basic test file

## CLI Usage

```bash
# Basic usage
rosetta_gen.py --input interface.yaml --lang js --output ./output

# Options
  -i, --input FILE       Input IDL file (required)
  -l, --lang LANG        Target language: js, python, all (required)
  -o, --output DIR       Output directory (default: ./bindings)
  -v, --verbose          Verbose output
  --dry-run              Parse but don't generate files
```

## Architecture

```
IDL File (YAML/JSON)
        ↓
    ILDParser
        ↓
InterfaceDescription (Python dataclass)
        ↓
    Language Generator (JSBindingGenerator, PythonBindingGenerator, etc.)
        ↓
Generated Binding Files
```

### Components

1. **ILDParser**: Parses YAML/JSON files into Python dataclasses
2. **InterfaceDescription**: Internal representation of the interface
3. **Language Generators**: Generate binding code for specific languages
4. **CLI Tool**: Command-line interface for the system

## Extending the System

### Adding a New Target Language

1. Create a new generator class (e.g., `CSharpBindingGenerator`)
2. Implement the required methods:
   - `generate_binding_code()`
   - `generate_build_config()`
   - `generate_test_file()`
3. Add the language to `rosetta_gen.py`

Example:

```python
class CSharpBindingGenerator:
    def __init__(self, interface: InterfaceDescription):
        self.interface = interface
    
    def generate_binding_code(self) -> str:
        # Generate C# P/Invoke code
        pass
    
    def generate_all(self, output_dir: str):
        # Generate all files
        pass
```

### Adding Custom Type Converters

For types not handled by built-in converters:

```yaml
converters:
  - type: "MyCustomType"
    kind: custom
```

Then manually register the converter in your binding code:

```cpp
gen.register_converter<MyCustomType>(
    // C++ to JS converter
    [](Napi::Env env, const core::Any &val) -> Napi::Value {
        // Implementation
    },
    // JS to C++ converter
    [](const Napi::Value &val) -> core::Any {
        // Implementation
    }
);
```

## Benefits

### Before (Manual Binding)

```cpp
// binding.cxx - manually written, ~200 lines
ROSETTA_REGISTER_CLASS(Vector3D)
    .field("x", &Vector3D::x)
    .field("y", &Vector3D::y)
    .field("z", &Vector3D::z)
    .method("length", &Vector3D::length)
    .method("normalize", &Vector3D::normalize);

BEGIN_JS_MODULE(gen) {
    register_classes();
    register_vector_converter<double>(gen);
    gen.bind_classes<Vector3D>();
    gen.add_utilities();
}
END_JS_MODULE()
```

### After (IDL-based)

```yaml
# interface.idl.yaml - declarative, ~30 lines
classes:
  - name: Vector3D
    fields:
      - { name: x, type: double }
      - { name: y, type: double }
      - { name: z, type: double }
    methods:
      - { name: length, returns: double, const: true }
      - { name: normalize }
```

```bash
# Generate bindings
rosetta_gen.py --input interface.idl.yaml --lang js --output ./bindings
```

## Future Enhancements

- [ ] Python bindings (PyBind11)
- [ ] C# bindings (P/Invoke)
- [ ] TypeScript type definitions
- [ ] Rust bindings (FFI)
- [ ] Documentation generation from IDL
- [ ] IDL validation and linting
- [ ] IDE support (syntax highlighting, autocomplete)
- [ ] Watch mode for development

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.