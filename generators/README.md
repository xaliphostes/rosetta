# Multi-Target Binding Generator

A C++ code generator that automatically produces language bindings for Python, JavaScript (Node.js), and WebAssembly from a single source of truth. Powered by [Rosetta](../include/rosetta), a C++ introspection library.

## Why Rosetta Matters

Traditional binding generation requires maintaining separate, hand-written wrapper code for each target language. This approach is:

- **Error-prone**: Each binding must be manually synchronized with the C++ API
- **Time-consuming**: Adding a new method requires updating bindings in 3+ places
- **Fragile**: API changes can silently break bindings

Rosetta solves this by providing **runtime introspection** for C++ classes. When you register a class with Rosetta, it captures complete metadata about:

- Class names and inheritance hierarchies
- Constructors and their parameter types
- Methods, their signatures, return types, and const-qualifiers
- Fields with getters and setters

This metadata becomes the **single source of truth** from which all bindings are generated automatically.

```cpp
// Register once with Rosetta
ROSETTA_REGISTER_CLASS(Surface)
    .constructor<>()
    .constructor<std::vector<Triangle>>()
    .method("area", &Surface::area)
    .method("triangles", &Surface::triangles);

// All bindings are generated automatically from this registration
```

## Architecture Overview

```
+------------------------------------------------------------------+
|                       Rosetta Registry                           |
|                                                                  |
|   +-----------+    +-----------+    +-----------+                |
|   |   Model   |    |  Surface  |    |  Solver   |    ...         |
|   |  metadata |    |  metadata |    |  metadata |                |
|   +-----------+    +-----------+    +-----------+                |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|                    MultiTargetGenerator                          |
|                                                                  |
|  +----------+ +----------+ +----------+ +----------+             |
|  |  Python  | |   WASM   | |JavaScript| | REST API |             |
|  +----------+ +----------+ +----------+ +----------+             |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|                        Output Files                              |
|                                                                  |
|  python/           wasm/           javascript/      rest/        |
|  |                 |               |                |            |
|  +- generated_     +- generated_   +- generated_    +- generated_|
|  |  pybind11.cxx   |  embind.cxx   |  napi.cxx      |  rest_api. |
|  +- CMakeLists     +- CMakeLists   +- package.json  +- CMakeLists|
|  +- setup.py       +- module.d.ts  +- binding.gyp   +- README.md |
|  +- module.pyi     +- README.md    +- index.js      |            |
|  +- README.md                      +- module.d.ts   |            |
+------------------------------------------------------------------+
```

## Generator Components

### Core Infrastructure

| File | Purpose |
|------|---------|
| `CodeWriter.h` | Base class providing indented code output and type mapping utilities |
| `TypeMapper.h` | Maps C++ types to Python/JavaScript/TypeScript equivalents |
| `TypeInfo.h` | Type information structures used by the mapper |
| `GeneratorConfig.h` | Configuration (module name, version, headers, include paths) |
| `MultiTargetGenerator.h` | Orchestrates all generators and manages output directories |

### Python Generators

| File | Output | Description |
|------|--------|-------------|
| `Pybind11Generator.h` | `generated_pybind11.cxx` | Generates pybind11 binding code. Queries Rosetta registry for all classes, performs topological sort for inheritance, and emits class definitions with constructors, methods, and NumPy conversions. |
| `PythonCMakeGenerator.h` | `CMakeLists.txt` | CMake build configuration for the Python extension module |
| `PythonSetupPyGenerator.h` | `setup.py` | Python setuptools configuration for `pip install` |
| `PythonPyprojectGenerator.h` | `pyproject.toml` | Modern Python packaging configuration (PEP 517/518) |
| `PythonStubGenerator.h` | `<module>.pyi` | Type stub file for IDE autocompletion and type checking |
| `PythonReadmeGenerator.h` | `README.md` | Documentation for the Python package |

### WebAssembly Generators

| File | Output | Description |
|------|--------|-------------|
| `EmbindGenerator.h` | `generated_embind.cxx` | Generates Emscripten embind code. Creates JavaScript-callable wrappers with TypedArray conversions for vectors and matrices. |
| `WasmCMakeGenerator.h` | `CMakeLists.txt` | CMake configuration for Emscripten compilation |
| `TypeScriptGenerator.h` | `<module>.d.ts` | TypeScript declarations for type-safe usage |
| `WasmReadmeGenerator.h` | `README.md` | Documentation for the WASM module |

### JavaScript/Node.js Generators

| File | Output | Description |
|------|--------|-------------|
| `NapiGenerator.h` | `generated_napi.cxx` | Generates N-API binding code using node-addon-api. Creates class wrappers with `Napi::ObjectWrap`, constructor overloads, method wrappers, and TypedArray conversions. |
| `JsPackageJsonGenerator.h` | `package.json` | npm package configuration |
| `JsBindingGypGenerator.h` | `binding.gyp` | node-gyp build configuration for native addon |
| `JsIndexGenerator.h` | `index.js` | Entry point that loads the native addon |
| `TypeScriptGenerator.h` | `<module>.d.ts` | TypeScript declarations (shared with WASM) |
| `JsReadmeGenerator.h` | `README.md` | Documentation for the npm package |

### REST API Generators

| File | Output | Description |
|------|--------|-------------|
| `RestApiGenerator.h` | `generated_rest_api.cxx` | Generates a complete REST API server using cpp-httplib. Exposes classes as HTTP endpoints with JSON request/response. Includes object lifecycle management, method invocation, and CORS support. |
| `RestApiCMakeGenerator.h` | `CMakeLists.txt` | CMake configuration with automatic dependency fetching (cpp-httplib, nlohmann_json) |
| `RestApiReadmeGenerator.h` | `README.md` | API documentation with endpoint descriptions and usage examples |

## How Rosetta Integration Works

### 1. Class Registration

Classes are registered with Rosetta at startup:

```cpp
// In registration.h
namespace arch_rosetta {
    void register_arch3_classes() {
        ROSETTA_REGISTER_CLASS(Surface)
            .constructor<>()
            .method("area", &Surface::area)
            .method("add_triangle", &Surface::add_triangle);
        
        ROSETTA_REGISTER_CLASS(Model)
            .constructor<std::string>()
            .method("solve", &Model::solve);
    }
}
```

### 2. Metadata Query

Generators query the Rosetta registry at generation time:

```cpp
auto& registry = rosetta::Registry::instance();

// Get all registered classes
for (const auto& name : registry.list_classes()) {
    auto* holder = registry.get_by_name(name);
    
    // Get class information
    std::string cpp_type = holder->get_cpp_type_name();
    std::string base_class = holder->get_base_class();
    
    // Iterate constructors
    for (const auto& ctor : holder->get_constructors()) {
        auto params = ctor.get_param_types();  // e.g., ["double", "std::string"]
    }
    
    // Iterate methods
    for (const auto& method_name : holder->get_methods()) {
        auto info = holder->get_method_info(method_name);
        // info.return_type, info.param_types, info.is_const
    }
}
```

### 3. Code Generation

Each generator transforms metadata into target-specific code:

```cpp
// Pybind11Generator emits:
py::class_<arch::Surface, std::shared_ptr<arch::Surface>>(m, "Surface")
    .def(py::init<>())
    .def("area", &arch::Surface::area)
    .def("add_triangle", &arch::Surface::add_triangle);

// EmbindGenerator emits:
class_<arch::Surface>("Surface")
    .constructor<>()
    .function("area", &arch::Surface::area)
    .function("addTriangle", &arch::Surface::add_triangle);
```

## Type Conversion

The `TypeMapper` handles automatic conversion between C++ types and target language types:

| C++ Type | Python | JavaScript | TypeScript |
|----------|--------|------------|------------|
| `int`, `long`, `size_t` | `int` | `number` | `number` |
| `float`, `double` | `float` | `number` | `number` |
| `std::string` | `str` | `string` | `string` |
| `std::vector<double>` | `numpy.ndarray` | `Float64Array` | `Float64Array` |
| `arch::Vector3` | `numpy.ndarray` | `Float64Array` | `Vector3` |
| `arch::Model` | `Model` | `Model` | `Model` |

Special conversion helpers are generated for compound types like `Vector3` and `Matrix33` to provide natural interfaces in each language (e.g., NumPy arrays in Python, TypedArrays in JavaScript).

## Usage

The binding generator needs access to your Rosetta metadata at generation time. There are two approaches:

### Approach 1: Custom Generator Binary (Recommended)

Create your own generator that links against your library:

```cpp
// my_generator.cpp
#include "BindingGeneratorLib.h"
#include "myproject/registration.h"  // Your registration header

int main(int argc, char* argv[]) {
    // Initialize Rosetta with your classes
    myproject_rosetta::register_classes();
    
    // Run the generator
    return BindingGeneratorLib::run(argc, argv);
}
```

Build this with your project linked, then run:

```bash
./my_generator project.json
```

### Approach 2: Template Generation

Use the standalone generator to create template files that you then customize:

```bash
./binding_generator --init project.json   # Create sample config
# Edit project.json with your settings
./binding_generator project.json          # Generate templates
```

### Configuration

Edit `project.json` to configure your project:

```json
{
    "project": {
        "name": "mylib",
        "version": "1.0.0"
    },
    "rosetta": {
        "registration_header": "src/registration.h",
        "registration_namespace": "mylib_rosetta",
        "registration_function": "register_classes",
        "types_namespace": "mylib"
    },
    "targets": {
        "python": { "enabled": true },
        "wasm": { "enabled": true },
        "javascript": { "enabled": false }
    }
}
```

This generates the complete binding structure:

```
output_dir/
â”œâ”€â”€ python/
â”‚   â”œâ”€â”€ generated_pybind11.cxx
â”‚   â”œâ”€â”€ CMakeLists.txt
â”‚   â”œâ”€â”€ setup.py
â”‚   â”œâ”€â”€ pyproject.toml
â”‚   â”œâ”€â”€ arch3.pyi
â”‚   â””â”€â”€ README.md
â”œâ”€â”€ wasm/
â”‚   â”œâ”€â”€ generated_embind.cxx
â”‚   â”œâ”€â”€ CMakeLists.txt
â”‚   â”œâ”€â”€ arch3.d.ts
â”‚   â””â”€â”€ README.md
â””â”€â”€ javascript/
    â”œâ”€â”€ package.json
    â”œâ”€â”€ binding.gyp
    â”œâ”€â”€ index.js
    â”œâ”€â”€ arch3.d.ts
    â””â”€â”€ README.md
```

## Building the Generated Bindings

### Python

```bash
cd output/python
pip install .
```

### WebAssembly

```bash
cd output/wasm
emcmake cmake -B build
cmake --build build
```

### Node.js

```bash
cd output/javascript
npm install
npm run build
```

## Benefits of This Approach

1. **Single Source of Truth**: The Rosetta registration defines the API once; all bindings derive from it.

2. **Automatic Synchronization**: When you add a method to Rosetta, it appears in all language bindings automatically.

3. **Type Safety**: Generated TypeScript declarations and Python stubs enable IDE autocompletion and static analysis.

4. **Consistent API**: All language bindings follow the same structure, making it easy to port code between languages.

5. **Runtime Introspection**: Generated modules expose `list_classes()` and `get_class_methods()` for dynamic exploration.

## Extending the Generator

To add a new target language:

1. Create a new generator class inheriting from `CodeWriter`
2. Query `rosetta::Registry::instance()` for class metadata
3. Emit binding code in the target format
4. Add the generator to `MultiTargetGenerator::generate_all()`

Example skeleton:

```cpp
class MyLanguageGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;
    
    void generate() override {
        auto& registry = rosetta::Registry::instance();
        
        for (const auto& name : registry.list_classes()) {
            auto* holder = registry.get_by_name(name);
            // Emit binding code for this class
        }
    }
};
```

## License

MIT

## ðŸ’¡ Credits

[Xaliphostes](https://github.com/xaliphostes) (fmaerten@gmail.com)
