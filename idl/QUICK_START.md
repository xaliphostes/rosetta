# Quick Start Guide - Rosetta ILD System

## 5-Minute Setup

### Prerequisites

- Python 3.7+
- Node.js 16+ (for JavaScript bindings)
- C++17 compiler
- PyYAML library

```bash
pip install pyyaml
```

### Step 1: Create Your Interface File

Create `mylib.ild.yaml`:

```yaml
module:
  name: mylib
  version: "1.0.0"

includes:
  - vector
  - optional
  - mylib/Calculator.h

converters:
  - type: "std::vector<double>"
    kind: vector
  - type: "std::optional<int>"
    kind: optional

classes:
  - name: Calculator
    fields:
      - name: result
        type: double
        access: ro
    
    methods:
      - name: add
        returns: double
        parameters:
          - name: a
            type: double
          - name: b
            type: double
      
      - name: multiply
        returns: double
        parameters:
          - name: a
            type: double
          - name: b
            type: double

utilities:
  version_info: true
  list_classes: true
  type_inspection: true
```

### Step 2: Generate Bindings

```bash
python rosetta_gen.py --input mylib.ild.yaml --lang js --output ./bindings
```

### Step 3: Build and Test

```bash
cd bindings/javascript
npm install
node test.js
```

## Common Patterns

### Simple Class with Fields

```yaml
classes:
  - name: Point2D
    fields:
      - { name: x, type: double, access: rw }
      - { name: y, type: double, access: rw }
```

### Class with Methods

```yaml
classes:
  - name: Vector3D
    fields:
      - { name: x, type: double }
      - { name: y, type: double }
      - { name: z, type: double }
    
    methods:
      - name: length
        returns: double
        const: true
      
      - name: normalize
        returns: void
```

### Complex Types

```yaml
classes:
  - name: DataStore
    fields:
      - name: values
        type: "std::vector<double>"
      
      - name: optional_value
        type: "std::optional<int>"
      
      - name: map_data
        type: "std::map<std::string, double>"

converters:
  - { type: "std::vector<double>", kind: vector }
  - { type: "std::optional<int>", kind: optional }
  - { type: "std::map<std::string, double>", kind: map }
```

### Read-Only and Write-Only Fields

```yaml
classes:
  - name: Sensor
    fields:
      - name: id
        type: int
        access: ro          # Read-only
      
      - name: calibration
        type: double
        access: wo          # Write-only
      
      - name: value
        type: double
        access: rw          # Read-write (default)
```

### Class with Inheritance

```yaml
classes:
  - name: Shape
    methods:
      - { name: area, returns: double, const: true }
  
  - name: Rectangle
    base_classes: [Shape]
    fields:
      - { name: width, type: double }
      - { name: height, type: double }
    methods:
      - { name: area, returns: double, const: true }
```

## CLI Options

```bash
# Basic usage
rosetta_gen.py --input interface.yaml --lang js --output ./out

# Verbose mode
rosetta_gen.py --input interface.yaml --lang js --output ./out --verbose

# Dry run (parse only, don't generate)
rosetta_gen.py --input interface.yaml --lang js --dry-run

# Generate for all languages
rosetta_gen.py --input interface.yaml --lang all --output ./out
```

## File Structure

After generation, you'll have:

```
bindings/
└── javascript/
    ├── binding.cxx      # Generated binding code
    ├── binding.gyp      # Build configuration
    ├── package.json     # NPM package
    └── test.js          # Test file
```

## Tips

1. **Start Simple**: Begin with a few classes and fields, then add complexity
2. **Use Verbose Mode**: `--verbose` helps debug parsing issues
3. **Test Incrementally**: Generate and test after each addition
4. **Read-Only for Safety**: Use `access: ro` for fields that shouldn't be modified from JS
5. **Document Types**: Use `description` fields for better generated documentation

## Troubleshooting

### "Module not found" error
```bash
pip install pyyaml --user
```

### "Class not registered" error
Make sure your C++ classes are defined before the binding code runs.

### Type conversion errors
Check that converters are registered for all complex types in your fields/methods.

## Next Steps

1. Read the full [README_ILD.md](README_ILD.md) for complete documentation
2. Check [SOLUTION_SUMMARY.md](SOLUTION_SUMMARY.md) for architecture details
3. Explore the example files in the repository
4. Try generating Python bindings (coming soon!)

## Example: Complete Workflow

```yaml
# geometry.ild.yaml
module:
  name: geometry
  version: "1.0.0"

converters:
  - { type: "std::vector<double>", kind: vector }

classes:
  - name: Vector3D
    description: "3D vector class"
    fields:
      - { name: x, type: double }
      - { name: y, type: double }
      - { name: z, type: double }
    methods:
      - name: length
        returns: double
        const: true
      - name: dot
        returns: double
        const: true
        parameters:
          - { name: other, type: Vector3D }

utilities:
  version_info: true
  list_classes: true
  type_inspection: true
```

```bash
# Generate bindings
python rosetta_gen.py --input geometry.ild.yaml --lang js --output ./bindings --verbose

# Build
cd bindings/javascript
npm install

# Test
node test.js

# Use in your code
const geometry = require('./build/Release/geometry.node');
const v1 = new geometry.Vector3D();
v1.x = 3;
v1.y = 4;
v1.z = 0;
console.log('Length:', v1.length()); // 5.0

const v2 = new geometry.Vector3D();
v2.x = 1;
v2.y = 0;
v2.z = 0;
console.log('Dot product:', v1.dot(v2)); // 3.0
```

That's it! You've successfully generated and used language bindings from a simple interface description.