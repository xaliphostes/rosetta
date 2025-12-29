# Binding Generator - Usage Guide

A code generator that produces language bindings (Python, JavaScript, WebAssembly, REST API) from C++ classes registered with Rosetta introspection.

## Quick Start

### 1. Configure Your Project

Create a `project.json` configuration file:

```json
{
    "project": {
        "name": "mylib",
        "version": "1.0.0",
        "description": "My C++ library bindings"
    },
    "rosetta": {
        "registration_header": "src/registration.h",
        "registration_namespace": "mylib_rosetta",
        "registration_function": "register_classes",
        "types_namespace": "mylib"
    },
    "includes": {
        "directories": [
            "include",
            "src",
            "../external/rosetta/include"
        ],
        "headers": [
            "mylib/Model.h",
            "mylib/Surface.h"
        ],
        "libraries": ["mylib_core", "rosetta"]
    },
    "output": {
        "base_dir": "./generated"
    },
    "targets": {
        "python": { "enabled": true },
        "wasm": { "enabled": true },
        "javascript": { "enabled": false },
        "rest": { "enabled": false }
    }
}
```

### 2. Include Paths

Your build system needs access to:

| Path | Purpose |
|------|---------|
| `rosetta/include` | Rosetta introspection library headers |
| Generator `.h` files directory | All generator headers (`Pybind11Generator.h`, `WasmGenerator.h`, etc.) |
| Your project's include directories | Headers for classes you're binding |

### 3. Create Your Generator Binary

**Why?** The generator needs to query Rosetta's registry at generation time. Since your classes are registered at runtime, you must link the generator against your library and call your registration function before generation.

Create `my_generator.cpp`:

```cpp
#include "BindingGeneratorLib.h"
#include "mylib/registration.h"  // Your Rosetta registration header

int main(int argc, char* argv[]) {
    // Register your classes with Rosetta FIRST
    mylib_rosetta::register_classes();
    
    // Run the generator
    return BindingGeneratorLib::run(argc, argv);
}
```

**Build the generator:**

```bash
mkdir build && cd build
cmake ..
make
```

### 4. Generate Bindings

```bash
# Generate all enabled targets
./my_generator project.json

# Other commands
./my_generator --init myconfig.json  # Create sample config
./my_generator --targets             # Show available targets
./my_generator --help                # Show help
```

### 5. Build the Generated Bindings

**Python:**
```bash
cd generated/python
pip install .
```

**WebAssembly:**
```bash
cd generated/wasm
emcmake cmake -B build && cmake --build build
```

**JavaScript (Node.js):**
```bash
cd generated/javascript
npm install && npm run build
```

**REST API:**
```bash
cd generated/rest
cmake -B build && cmake --build build
./mylib_server --port 8080
```

## Configuration Reference

### `rosetta` Section

| Field | Description |
|-------|-------------|
| `registration_header` | Path to your registration header file |
| `registration_namespace` | Namespace containing `register_classes()` |
| `registration_function` | Function name to call for registration |
| `types_namespace` | Namespace prefix for your bound types |

### `targets` Section

Each target supports:
- `enabled`: Enable/disable generation
- `output_dir`: Override default output directory
- `extra_sources`: Additional source files to include
- `extra_libs`: Additional libraries to link

WASM-specific options:
- `single_file`: Embed WASM in JS file
- `export_es6`: Generate ES6 module
- `environment`: Target environment (`"web"`, `"node"`, `"web,node"`)

## Output Structure

```
generated/
├── python/
│   ├── generated_pybind11.cxx
│   ├── CMakeLists.txt
│   ├── example.py
│   ├── setup.py
│   ├── mylib.pyi
│   └── README.md
├── wasm/
│   ├── generated_embind.cxx
│   ├── CMakeLists.txt
│   ├── example.js
│   ├── mylib.d.ts
│   └── README.md
├── javascript/
│   ├── generated_napi.cxx
│   ├── package.json
│   ├── example.js
│   ├── binding.gyp
│   └── mylib.d.ts
└── rest/
    ├── generated_rest_api.cxx
    ├── CMakeLists.txt
    ├── index.html    
    └── README.md
```

## Example Registration Header

Your `registration.h` should use Rosetta macros:

```cpp
#pragma once
#include <rosetta/rosetta.h>
#include "mylib/Model.h"
#include "mylib/Surface.h"

namespace mylib_rosetta {
    inline void register_classes() {
        ROSETTA_REGISTER_CLASS(mylib::Model)
            .constructor<>()
            .constructor<std::string>()
            .method("compute", &mylib::Model::compute)
            .method("getResult", &mylib::Model::getResult)
            .field("name", &mylib::Model::name);

        ROSETTA_REGISTER_CLASS(mylib::Surface)
            .constructor<std::vector<double>, std::vector<int>>()
            .method("area", &mylib::Surface::area)
            .property("points", &mylib::Surface::getPoints);
    }
}
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "No classes registered" | Ensure your registration function is called before `BindingGeneratorLib::run()` |
| Missing types in bindings | Add headers to `includes.headers` in config |
| Link errors | Check `includes.libraries` and library paths |
| WASM build fails | Ensure Emscripten is properly installed: `emcmake cmake ...` |