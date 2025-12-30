# Rosetta Binding Generator - Configuration Guide

## Overview

The `project.json` file is the central configuration file for the Rosetta Binding Generator. It defines how your C++ library will be exposed to other languages (Python, JavaScript/Node.js, WebAssembly, and REST API).

This guide covers all available options, their purposes, and how to use them effectively.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Configuration Sections](#configuration-sections)
   - [Variables](#variables)
   - [Project Metadata](#project-metadata)
   - [Rosetta Configuration](#rosetta-configuration)
   - [Includes](#includes)
   - [Output](#output)
   - [Targets](#targets)
   - [Options](#options)
   - [Advanced](#advanced)
3. [Complete Reference](#complete-reference)
4. [Examples](#examples)
5. [Troubleshooting](#troubleshooting)

---

## Quick Start

Create a minimal `project.json`:

```json
{
    "project": {
        "name": "mylib",
        "version": "1.0.0"
    },
    "rosetta": {
        "registration_header": "src/bindings/registration.h",
        "registration_function": "register_classes"
    },
    "includes": {
        "directories": ["include"]
    },
    "output": {
        "base_dir": "./generated"
    },
    "targets": {
        "python": { "enabled": true }
    }
}
```

Generate bindings:
```bash
./binding_generator project.json
```

---

## Configuration Sections

### Variables

**Purpose:** Define reusable variables that can be substituted throughout the configuration using `${VAR_NAME}` syntax.

```json
{
    "variables": {
        "PROJECT_ROOT": "/path/to/your/project",
        "ROSETTA_ROOT": "/path/to/rosetta",
        "BUILD_DIR": "${PROJECT_ROOT}/build"
    }
}
```

**Features:**
- Variables are substituted recursively (a variable can reference another variable)
- Environment variables can be accessed using `${env:VAR_NAME}` syntax
- Variables can be used anywhere in the configuration

**Example with environment variables:**
```json
{
    "variables": {
        "HOME_DIR": "${env:HOME}",
        "PROJECT_ROOT": "${HOME_DIR}/projects/mylib"
    }
}
```

---

### Project Metadata

**Purpose:** Define project identification and metadata used in generated files (package.json, setup.py, README files, etc.).

```json
{
    "project": {
        "name": "myproject",
        "version": "1.0.0",
        "description": "My awesome C++ library with auto-generated bindings",
        "author": "Your Name",
        "license": "MIT"
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `name` | string | `"myproject"` | **Required.** Project/module name. Used as the Python module name, npm package name, etc. Must be a valid identifier (no spaces or special characters). |
| `version` | string | `"1.0.0"` | Semantic version string. Used in generated package files. |
| `description` | string | `"C++ bindings..."` | Human-readable description for package managers. |
| `author` | string | `"Generated"` | Author name for attribution. |
| `license` | string | `"MIT"` | License identifier (e.g., "MIT", "Apache-2.0", "GPL-3.0"). |

---

### Rosetta Configuration

**Purpose:** Configure how the generator connects to your Rosetta class registration.

```json
{
    "rosetta": {
        "registration_header": "src/bindings/registration.h",
        "registration_namespace": "mylib_bindings",
        "registration_function": "register_classes",
        "types_namespace": "mylib",
        "cpp_namespaces": {
            "strip": true,
            "separator": ""
        }
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `registration_header` | string | — | **Required.** Path to the header file containing your Rosetta registration code. This file will be copied to each target's output directory. |
| `registration_namespace` | string | `""` | C++ namespace containing the registration function. Leave empty if the function is in the global namespace. |
| `registration_function` | string | — | **Required.** Name of the function that registers all classes with Rosetta (e.g., `register_classes`). |
| `types_namespace` | string | `""` | Default namespace for your bound types. Used for type resolution in generated code. |
| `cpp_namespaces` | object | — | Controls how C++ namespaces are handled in generated binding names. |

#### cpp_namespaces Options

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `strip` | bool | `true` | If `true`, removes namespace prefixes from generated binding names. `mylib::Vector3` becomes `Vector3`. |
| `separator` | string | `""` | Character(s) to replace `::` when not stripping. Empty means complete removal. Use `"_"` to get `mylib_Vector3`. |

**Examples:**

| C++ Class | strip=true, sep="" | strip=true, sep="_" | strip=false, sep="_" |
|-----------|-------------------|---------------------|----------------------|
| `mylib::Vector3` | `Vector3` | `Vector3` | `mylib_Vector3` |
| `mylib::math::Matrix` | `Matrix` | `Matrix` | `mylib_math_Matrix` |

---

### Includes

**Purpose:** Specify paths and dependencies needed to compile the generated bindings.

```json
{
    "includes": {
        "directories": [
            "${PROJECT_ROOT}/include",
            "${PROJECT_ROOT}/extern/eigen",
            "${ROSETTA_ROOT}/include"
        ],
        "library_directories": [
            "${PROJECT_ROOT}/build/lib"
        ],
        "headers": [
            "mylib/core/Types.h",
            "mylib/core/Model.h"
        ],
        "libraries": [
            "mylib_core",
            "pthread"
        ]
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `directories` | string[] | `[]` | Include paths (`-I` flags). Can be absolute or relative to the config file. |
| `library_directories` | string[] | `[]` | Library search paths (`-L` flags). |
| `headers` | string[] | `[]` | Additional headers to `#include` in generated code. |
| `libraries` | string[] | `[]` | Libraries to link against (`-l` flags, without the `-l` prefix). |

**Path Resolution:**
- Relative paths are resolved relative to the `project.json` file location
- Use `${VAR}` syntax to reference variables
- Paths are automatically normalized for the target platform

---

### Output

**Purpose:** Configure where generated files are written.

```json
{
    "output": {
        "base_dir": "./generated"
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `base_dir` | string | `"./generated"` | Base directory for all generated files. Each target creates a subdirectory here (e.g., `./generated/python/`). |

**Directory Structure Created:**
```
generated/
├── python/
│   ├── generated_pybind11.cxx
│   ├── CMakeLists.txt
│   ├── setup.py
│   ├── pyproject.toml
│   ├── myproject.pyi
│   ├── README.md
│   └── example.py
├── wasm/
│   ├── generated_embind.cxx
│   ├── CMakeLists.txt
│   ├── myproject.d.ts
│   ├── README.md
│   └── example.js
├── javascript/
│   ├── generated_napi.cxx
│   ├── package.json
│   ├── binding.gyp
│   ├── index.js
│   ├── myproject.d.ts
│   ├── README.md
│   └── example.js
└── rest/
    ├── generated_rest_api.cxx
    ├── CMakeLists.txt
    ├── README.md
    └── index.html
```

---

### Targets

**Purpose:** Enable and configure specific binding targets.

```json
{
    "targets": {
        "python": {
            "enabled": true,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": []
        },
        "wasm": {
            "enabled": true,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": [],
            "single_file": true,
            "export_es6": false,
            "environment": "web,node"
        },
        "javascript": {
            "enabled": false,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": []
        },
        "rest": {
            "enabled": false,
            "output_dir": "",
            "extra_sources": [],
            "extra_libs": []
        }
    }
}
```

#### Common Target Options

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `enabled` | bool | `false` | Whether to generate this target. |
| `output_dir` | string | `""` | Override the default output directory. If empty, uses `{base_dir}/{target_name}/`. |
| `extra_sources` | string[] | `[]` | Additional C++ source files to compile with the bindings. |
| `extra_libs` | string[] | `[]` | Additional libraries to link (beyond those in `includes.libraries`). |

#### Python Target

Uses **pybind11** to generate native Python extensions.

**Generated Files:**
- `generated_pybind11.cxx` — Main binding code
- `CMakeLists.txt` — CMake build configuration
- `setup.py` — setuptools configuration
- `pyproject.toml` — PEP 517/518 build configuration
- `{name}.pyi` — Type stub file for IDE support
- `README.md` — Usage documentation
- `example.py` — Example usage script

**Build:**
```bash
cd generated/python
pip install .
# or
cmake -B build && cmake --build build
```

#### WASM Target

Uses **Emscripten/embind** to generate WebAssembly modules.

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `single_file` | bool | `false` | Embed the `.wasm` binary directly in the JavaScript file (larger file, simpler deployment). |
| `export_es6` | bool | `false` | Generate ES6 module syntax instead of CommonJS. |
| `environment` | string | `""` | Target environment: `"web"`, `"node"`, or `"web,node"` for both. |

**Generated Files:**
- `generated_embind.cxx` — Embind binding code
- `CMakeLists.txt` — CMake configuration for Emscripten
- `{name}.d.ts` — TypeScript declarations
- `README.md` — Usage documentation
- `example.js` — Example usage script

**Build:**
```bash
cd generated/wasm
emcmake cmake -B build
cmake --build build
```

#### JavaScript Target

Uses **Node.js N-API** to generate native Node.js addons.

**Generated Files:**
- `generated_napi.cxx` — N-API binding code
- `package.json` — npm package configuration
- `binding.gyp` — node-gyp build configuration
- `index.js` — ES module entry point
- `{name}.d.ts` — TypeScript declarations
- `README.md` — Usage documentation
- `example.js` — Example usage script

**Build:**
```bash
cd generated/javascript
npm install
npm run build
```

#### REST Target

Generates a standalone REST API server using **cpp-httplib**.

**Generated Files:**
- `generated_rest_api.cxx` — Complete REST server implementation
- `CMakeLists.txt` — CMake build configuration
- `README.md` — API documentation
- `index.html` — Interactive API explorer

**API Endpoints:**
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/classes` | List all registered classes |
| GET | `/api/classes/:name` | Get class metadata |
| POST | `/api/objects/:class` | Create new object |
| POST | `/api/objects/:id/:method` | Call method on object |
| DELETE | `/api/objects/:id` | Delete object |

**Build & Run:**
```bash
cd generated/rest
cmake -B build && cmake --build build
./myproject_server --port 8080
```

---

### Options

**Purpose:** Control which auxiliary files are generated.

```json
{
    "options": {
        "generate_stubs": true,
        "generate_typescript": true,
        "generate_readme": true,
        "generate_example": true,
        "generate_cmake": true
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `generate_stubs` | bool | `true` | Generate Python type stub files (`.pyi`) for IDE autocompletion and type checking. |
| `generate_typescript` | bool | `true` | Generate TypeScript declaration files (`.d.ts`) for JS/WASM targets. |
| `generate_readme` | bool | `true` | Generate `README.md` with build instructions and usage examples for each target. |
| `generate_example` | bool | `true` | Generate example scripts (`example.py`, `example.js`, `index.html`). |
| `generate_cmake` | bool | `true` | Generate `CMakeLists.txt` for targets that use CMake (Python, WASM, REST). |

---

### Advanced

**Purpose:** Fine-tune binding generation behavior.

```json
{
    "advanced": {
        "numpy_types": [
            "Vector3",
            "Matrix33",
            "std::vector<double>",
            "std::vector<float>",
            "std::vector<int>"
        ],
        "skip_classes": [
            "InternalHelper",
            "DebugUtility"
        ],
        "skip_methods": [
            "MyClass::internalMethod",
            "debugDump"
        ]
    }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `numpy_types` | string[] | `["Vector3", "Matrix33", "std::vector<double>"]` | Types that should be automatically converted to NumPy arrays (Python) or TypedArrays (JavaScript). |
| `skip_classes` | string[] | `[]` | Classes to exclude from binding generation. Useful for internal/helper classes. |
| `skip_methods` | string[] | `[]` | Methods to exclude. Can be fully qualified (`ClassName::method`) or just method names to skip globally. |

#### numpy_types Details

Types listed here receive special treatment:
- **Python:** Converted to/from `numpy.ndarray`
- **JavaScript/WASM:** Converted to/from `TypedArray` (`Float64Array`, `Int32Array`, etc.)

The generator matches type names using substring matching, so `"std::vector<double>"` will match `const std::vector<double>&`, `std::vector<double>*`, etc.

---

## Complete Reference

### Full project.json Schema

```json
{
    "variables": {
        "VAR_NAME": "value"
    },
    "project": {
        "name": "string (required)",
        "version": "string",
        "description": "string",
        "author": "string",
        "license": "string"
    },
    "rosetta": {
        "registration_header": "string (required)",
        "registration_namespace": "string",
        "registration_function": "string (required)",
        "types_namespace": "string",
        "cpp_namespaces": {
            "strip": "boolean",
            "separator": "string"
        }
    },
    "includes": {
        "directories": ["string"],
        "library_directories": ["string"],
        "headers": ["string"],
        "libraries": ["string"]
    },
    "output": {
        "base_dir": "string"
    },
    "targets": {
        "python": {
            "enabled": "boolean",
            "output_dir": "string",
            "extra_sources": ["string"],
            "extra_libs": ["string"]
        },
        "wasm": {
            "enabled": "boolean",
            "output_dir": "string",
            "extra_sources": ["string"],
            "extra_libs": ["string"],
            "single_file": "boolean",
            "export_es6": "boolean",
            "environment": "string"
        },
        "javascript": {
            "enabled": "boolean",
            "output_dir": "string",
            "extra_sources": ["string"],
            "extra_libs": ["string"]
        },
        "rest": {
            "enabled": "boolean",
            "output_dir": "string",
            "extra_sources": ["string"],
            "extra_libs": ["string"]
        }
    },
    "options": {
        "generate_stubs": "boolean",
        "generate_typescript": "boolean",
        "generate_readme": "boolean",
        "generate_example": "boolean",
        "generate_cmake": "boolean"
    },
    "advanced": {
        "numpy_types": ["string"],
        "skip_classes": ["string"],
        "skip_methods": ["string"]
    }
}
```

---

## Examples

### Minimal Python-Only Configuration

```json
{
    "project": {
        "name": "mymath"
    },
    "rosetta": {
        "registration_header": "bindings/register.h",
        "registration_function": "register_all"
    },
    "includes": {
        "directories": ["include"]
    },
    "targets": {
        "python": { "enabled": true }
    }
}
```

### Multi-Target Scientific Library

```json
{
    "variables": {
        "ROOT": "/home/user/projects/scilib",
        "EIGEN": "/usr/include/eigen3"
    },
    "project": {
        "name": "scilib",
        "version": "2.1.0",
        "description": "Scientific computing library",
        "author": "Science Team",
        "license": "BSD-3-Clause"
    },
    "rosetta": {
        "registration_header": "${ROOT}/src/bindings/rosetta_registration.h",
        "registration_namespace": "scilib::bindings",
        "registration_function": "register_scilib",
        "types_namespace": "scilib",
        "cpp_namespaces": {
            "strip": true,
            "separator": ""
        }
    },
    "includes": {
        "directories": [
            "${ROOT}/include",
            "${EIGEN}",
            "/usr/local/include"
        ],
        "library_directories": [
            "${ROOT}/build/lib"
        ],
        "libraries": ["scilib", "blas", "lapack"]
    },
    "output": {
        "base_dir": "${ROOT}/bindings/generated"
    },
    "targets": {
        "python": {
            "enabled": true,
            "extra_libs": ["numpy"]
        },
        "wasm": {
            "enabled": true,
            "single_file": true,
            "environment": "web,node"
        },
        "javascript": {
            "enabled": true
        }
    },
    "advanced": {
        "numpy_types": [
            "scilib::Vector",
            "scilib::Matrix",
            "std::vector<double>",
            "std::vector<std::complex<double>>"
        ],
        "skip_classes": ["InternalBuffer", "DebugHelper"],
        "skip_methods": ["Matrix::unsafeData", "debugPrint"]
    }
}
```

### REST API Server Only

```json
{
    "project": {
        "name": "myservice",
        "version": "1.0.0",
        "description": "REST API for MyLib"
    },
    "rosetta": {
        "registration_header": "src/registration.h",
        "registration_function": "init_rosetta"
    },
    "includes": {
        "directories": ["include", "extern/httplib"],
        "libraries": ["mylib", "pthread"]
    },
    "output": {
        "base_dir": "./api_server"
    },
    "targets": {
        "rest": {
            "enabled": true
        }
    },
    "options": {
        "generate_cmake": true,
        "generate_readme": true,
        "generate_example": true
    }
}
```

### Custom Output Directories

```json
{
    "project": { "name": "mylib" },
    "rosetta": {
        "registration_header": "registration.h",
        "registration_function": "register_types"
    },
    "output": {
        "base_dir": "./build/bindings"
    },
    "targets": {
        "python": {
            "enabled": true,
            "output_dir": "/opt/python-packages/mylib"
        },
        "wasm": {
            "enabled": true,
            "output_dir": "./web/wasm"
        }
    }
}
```

---

## Troubleshooting

### Common Errors

#### "Config file not found"
```
Error: Config file not found: project.json
```
**Solution:** Ensure the path to your config file is correct. Use absolute path if needed.

#### "Missing rosetta.registration_header"
```
Error: Invalid configuration
  - Missing rosetta.registration_header
```
**Solution:** Add the required `registration_header` field pointing to your Rosetta registration code.

#### "No targets enabled"
```
Error: Invalid configuration
  - No targets enabled
```
**Solution:** Enable at least one target in the `targets` section.

#### "Registration header not found"
```
Warning: Registration header not found: path/to/header.h
```
**Solution:** Check that the path in `registration_header` is correct. Relative paths are resolved from the config file's directory.

### Build Issues

#### Python: "pybind11 not found"
Install pybind11:
```bash
pip install pybind11
# or
conda install -c conda-forge pybind11
```

#### WASM: "emcmake not found"
Install and activate Emscripten:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

#### JavaScript: "node-gyp failed"
Ensure you have build tools installed:
```bash
# macOS
xcode-select --install

# Ubuntu/Debian
sudo apt-get install build-essential

# Windows
npm install -g windows-build-tools
```

### Tips

1. **Start simple:** Begin with a single target (Python is easiest) and add more once working.

2. **Use variables:** For complex projects, define paths as variables to keep the config DRY.

3. **Test incrementally:** After generating, try building each target before adding more complexity.

4. **Check paths:** Most issues come from incorrect paths. Use absolute paths during debugging.

5. **Inspect generated code:** If bindings don't work as expected, examine the generated `.cxx` files.

---

## Command Line Reference

```bash
# Generate from config file
./binding_generator project.json

# Create sample config
./binding_generator --init [path]

# Show available targets with details
./binding_generator --targets

# Show help
./binding_generator --help
```

---

## Version History

- **1.0.0** — Initial release with Python, WASM, JavaScript, and REST targets
