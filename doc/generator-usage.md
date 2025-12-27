# Binding Generator - Usage Guide

This guide shows how to use the Multi-Target Binding Generator to create Python, WebAssembly, and Node.js bindings from your C++ library.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Project Configuration](#project-configuration)
3. [Integration with Your Project](#integration-with-your-project)
4. [Building the Generated Bindings](#building-the-generated-bindings)
5. [Using the Bindings](#using-the-bindings)
6. [Advanced Configuration](#advanced-configuration)

---

## Quick Start

### Step 1: Create a Configuration File

```bash
./binding_generator --init myproject.json
```

This creates a sample `myproject.json` configuration file.

### Step 2: Edit the Configuration

```json
{
    "project": {
        "name": "mylib",
        "version": "1.0.0",
        "description": "My awesome C++ library",
        "author": "Your Name",
        "license": "MIT"
    },
    "rosetta": {
        "registration_header": "src/bindings/registration.h",
        "registration_namespace": "mylib_rosetta",
        "registration_function": "register_classes",
        "types_namespace": "mylib"
    },
    "includes": {
        "directories": ["include", "src"],
        "headers": [
            "mylib/Vector3.h",
            "mylib/Matrix33.h",
            "mylib/Model.h"
        ],
        "libraries": ["mylib_core"]
    },
    "targets": {
        "python": { "enabled": true },
        "wasm": { "enabled": true },
        "javascript": { "enabled": true }
    }
}
```

### Step 3: Create Your Rosetta Registration

In `src/bindings/registration.h`:

```cpp
#pragma once
#include <rosetta/rosetta.h>
#include "mylib/Vector3.h"
#include "mylib/Model.h"

namespace mylib_rosetta {

void register_classes() {
    using namespace mylib;

    ROSETTA_REGISTER_CLASS(Vector3)
        .constructor<>()
        .constructor<double, double, double>()
        .method("x", &Vector3::x)
        .method("y", &Vector3::y)
        .method("z", &Vector3::z)
        .method("length", &Vector3::length)
        .method("normalize", &Vector3::normalize);

    ROSETTA_REGISTER_CLASS(Model)
        .constructor<>()
        .constructor<std::string>()
        .method("load", &Model::load)
        .method("save", &Model::save)
        .method("compute", &Model::compute)
        .method("get_result", &Model::get_result);
}

} // namespace mylib_rosetta
```

### Step 4: Create a Custom Generator

Create `my_generator.cpp`:

```cpp
#include "BindingGeneratorLib.h"
#include "src/bindings/registration.h"

int main(int argc, char* argv[]) {
    // Initialize Rosetta with your classes
    mylib_rosetta::register_classes();
    
    // Run the generator
    return BindingGeneratorLib::run(argc, argv);
}
```

### Step 5: Build and Run

```bash
# Build your custom generator
g++ -std=c++20 -I/path/to/binding_generator -I/path/to/rosetta \
    my_generator.cpp -o my_generator -lmylib_core

# Generate bindings
./my_generator myproject.json
```

### Step 6: Check the Output

```
generated/
├── python/
│   ├── generated_pybind11.cxx
│   ├── CMakeLists.txt
│   ├── setup.py
│   ├── pyproject.toml
│   ├── mylib.pyi
│   └── README.md
├── wasm/
│   ├── generated_embind.cxx
│   ├── CMakeLists.txt
│   ├── mylib.d.ts
│   └── README.md
├── javascript/
│   ├── generated_napi.cxx
│   ├── package.json
│   ├── binding.gyp
│   ├── index.js
│   ├── mylib.d.ts
│   └── README.md
└── rest/
    ├── generated_rest_api.cxx
    ├── CMakeLists.txt
    └── README.md
```

---

## Project Configuration

### Full Configuration Reference

```json
{
    "project": {
        "name": "mylib",
        "version": "1.0.0",
        "description": "Description of your library",
        "author": "Your Name",
        "license": "MIT"
    },
    
    "rosetta": {
        "registration_header": "path/to/registration.h",
        "registration_namespace": "mylib_rosetta",
        "registration_function": "register_classes",
        "types_namespace": "mylib"
    },
    
    "includes": {
        "directories": ["include", "src", "../external"],
        "headers": [
            "mylib/Types.h",
            "mylib/Model.h"
        ],
        "libraries": ["mylib_core", "mylib_math"]
    },
    
    "output": {
        "base_dir": "./generated"
    },
    
    "targets": {
        "python": {
            "enabled": true,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": ["python_specific_lib"]
        },
        "wasm": {
            "enabled": true,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": []
        },
        "javascript": {
            "enabled": false,
            "output_dir": "./custom_js_output",
            "extra_sources": [],
            "extra_libs": []
        },
        "rest": {
            "enabled": true,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": []
        }
    },
    
    "options": {
        "generate_stubs": true,
        "generate_typescript": true,
        "generate_readme": true,
        "generate_cmake": true
    },
    
    "advanced": {
        "numpy_types": [
            "Vector3",
            "Matrix33",
            "std::vector<double>"
        ],
        "skip_classes": ["InternalHelper"],
        "skip_methods": ["Model::internal_method", "debug_dump"]
    }
}
```

### Configuration Fields

| Section | Field | Description |
|---------|-------|-------------|
| `project` | `name` | Module name (used for imports) |
| `project` | `version` | Semantic version string |
| `rosetta` | `registration_header` | Path to your Rosetta registration file |
| `rosetta` | `registration_namespace` | C++ namespace containing the registration function |
| `rosetta` | `registration_function` | Function name that registers classes |
| `rosetta` | `types_namespace` | C++ namespace of your types (e.g., `mylib`) |
| `includes` | `directories` | Include paths for compilation |
| `includes` | `headers` | Headers to include in generated code |
| `includes` | `libraries` | Libraries to link against |
| `targets` | `enabled` | Whether to generate this target |
| `targets` | `output_dir` | Custom output directory (empty = default) |
| `advanced` | `numpy_types` | Types to convert to NumPy/TypedArray |
| `advanced` | `skip_classes` | Classes to exclude from bindings |
| `advanced` | `skip_methods` | Methods to exclude (format: `Class::method`) |

---

## Integration with Your Project

### CMake Integration

Add to your `CMakeLists.txt`:

```cmake
# Build the binding generator
add_executable(generate_bindings
    tools/generate_bindings.cpp
)

target_include_directories(generate_bindings PRIVATE
    ${CMAKE_SOURCE_DIR}/binding_generator
    ${ROSETTA_INCLUDE_DIR}
)

target_link_libraries(generate_bindings PRIVATE
    mylib_core
    nlohmann_json::nlohmann_json
)

# Custom target to regenerate bindings
add_custom_target(regenerate_bindings
    COMMAND generate_bindings ${CMAKE_SOURCE_DIR}/project.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Regenerating language bindings..."
)
```

### Header-Only Integration

If you don't want to build a separate generator, include all headers directly:

```cpp
#include "BindingGeneratorLib.h"
#include "MultiTargetGenerator.h"
#include "your_registration.h"

int main() {
    your_namespace::register_classes();
    
    ProjectConfig config;
    config.name = "mylib";
    config.version = "1.0.0";
    config.registration_header = "registration.h";
    config.registration_namespace = "your_namespace";
    config.registration_function = "register_classes";
    config.types_namespace = "mylib";
    config.python.enabled = true;
    config.wasm.enabled = true;
    config.output_base_dir = "./generated";
    
    return BindingGeneratorLib::generate(config);
}
```

---

## Building the Generated Bindings

### Python

```bash
cd generated/python

# Option 1: pip install
pip install .

# Option 2: CMake build
mkdir build && cd build
cmake ..
make
```

### WebAssembly

```bash
cd generated/wasm

# Requires Emscripten SDK
source /path/to/emsdk/emsdk_env.sh

mkdir build && cd build
emcmake cmake ..
emmake make
```

### Node.js / JavaScript

```bash
cd generated/javascript

npm install
npm run build
```

### REST API Server

```bash
cd generated/rest

mkdir build && cd build
cmake ..
make

# Run the server
./mylib_server --port 8080
```

---

## Using the Bindings

### Python Example

```python
import mylib
import numpy as np

# List available classes
print(mylib.list_classes())
# Output: ['Vector3', 'Model', ...]

# Create objects
v = mylib.Vector3(1.0, 2.0, 3.0)
print(f"Length: {v.length()}")

# NumPy integration (for configured types)
result = model.compute()  # Returns numpy array
print(result.shape)
```

### JavaScript (Node.js) Example

```javascript
import mylib from './mylib/index.js';

// List available classes
console.log(mylib.listClasses());

// Create objects
const model = new mylib.Model("config.json");
model.load();

// TypedArray integration
const result = model.compute();  // Returns Float64Array
console.log(result.length);
```

### WebAssembly Example

```javascript
import createMylib from './mylib.js';

async function main() {
    const mylib = await createMylib();
    
    // List available classes
    console.log(mylib.listClasses());
    
    // Create objects
    const model = new mylib.Model();
    model.load("data.json");
    
    // Don't forget to clean up!
    model.delete();
}

main();
```

### TypeScript Example

```typescript
import createMylib, { Model, Vector3 } from './mylib.js';

async function main(): Promise<void> {
    const mylib = await createMylib();
    
    const v: Vector3 = new Float64Array([1, 2, 3]);
    const model: Model = new mylib.Model();
    
    model.compute();
    model.delete();
}
```

### REST API Example (curl)

```bash
# List available classes
curl http://localhost:8080/api/classes

# Get class information
curl http://localhost:8080/api/classes/Model

# Create an object
curl -X POST http://localhost:8080/api/objects/Model \
  -H "Content-Type: application/json" \
  -d '["config.json"]'
# Response: {"error":false,"data":{"id":"Model_1","class":"Model"}}

# Call a method
curl -X POST http://localhost:8080/api/objects/Model_1/compute

# Call a method with arguments
curl -X POST http://localhost:8080/api/objects/Model_1/set_value \
  -H "Content-Type: application/json" \
  -d '[42.5]'

# List all objects
curl http://localhost:8080/api/objects

# Delete an object
curl -X DELETE http://localhost:8080/api/objects/Model_1
```

### REST API Example (JavaScript)

```javascript
const API = 'http://localhost:8080/api';

// Create object
const res = await fetch(`${API}/objects/Model`, {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify(['config.json'])
});
const { data: { id } } = await res.json();

// Call method
await fetch(`${API}/objects/${id}/compute`, { method: 'POST' });

// Get result
const result = await fetch(`${API}/objects/${id}/get_result`, { method: 'POST' });
console.log(await result.json());

// Clean up
await fetch(`${API}/objects/${id}`, { method: 'DELETE' });
```

### REST API Example (Python)

```python
import requests

API = 'http://localhost:8080/api'

# Create object
res = requests.post(f'{API}/objects/Model', json=['config.json'])
obj_id = res.json()['data']['id']

# Call method
requests.post(f'{API}/objects/{obj_id}/compute')

# Get result  
result = requests.post(f'{API}/objects/{obj_id}/get_result')
print(result.json()['data'])

# Clean up
requests.delete(f'{API}/objects/{obj_id}')
```

---

## Advanced Configuration

### Custom Type Mappings

Types listed in `numpy_types` are automatically converted:
- **Python**: Converts to/from `numpy.ndarray`
- **JavaScript/WASM**: Converts to/from `Float64Array` / `Int32Array`

```json
{
    "advanced": {
        "numpy_types": [
            "Vector3",
            "Matrix33",
            "Quaternion",
            "std::vector<double>",
            "std::vector<float>",
            "std::vector<int>"
        ]
    }
}
```

### Skipping Classes and Methods

```json
{
    "advanced": {
        "skip_classes": [
            "InternalHelper",
            "DebugTools"
        ],
        "skip_methods": [
            "Model::_internal_init",
            "Model::debug_dump",
            "get_raw_pointer"
        ]
    }
}
```

### Custom Output Directories

```json
{
    "targets": {
        "python": {
            "enabled": true,
            "output_dir": "/path/to/python/package/src"
        },
        "wasm": {
            "enabled": true,
            "output_dir": "../web/wasm"
        }
    }
}
```

### Selective Target Generation

Generate only specific targets:

```json
{
    "targets": {
        "python": { "enabled": true },
        "wasm": { "enabled": false },
        "javascript": { "enabled": false }
    }
}
```

---

## Troubleshooting

### "Class not found in registry"

Make sure your registration function is called before running the generator:

```cpp
int main(int argc, char* argv[]) {
    mylib_rosetta::register_classes();  // Must be called first!
    return BindingGeneratorLib::run(argc, argv);
}
```

### "Cannot find header file"

Check that `includes.directories` paths are correct and relative to the config file location.

### Python import errors

Ensure the compiled module is in your Python path:

```bash
export PYTHONPATH=$PYTHONPATH:/path/to/generated/python/build
```

### WASM module loading issues

Make sure both `.js` and `.wasm` files are served from the same directory with correct MIME types.

---

## License

MIT

## Credits

[Xaliphostes](https://github.com/xaliphostes) (fmaerten@gmail.com)
