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
- Fields (direct access to member variables)
- Properties (virtual fields) given by (getters, setters) or just a getter (readonly)

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
|  |  Python  | |   WASM   | |JavaScript| | REST API |   ...       |
|  +----------+ +----------+ +----------+ +----------+             |
+------------------------------------------------------------------+
```

## Usage

The binding generator needs access to your Rosetta metadata at generation time.

### Generate the Custom Generator Binary

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

### Configuration

Edit `project.json` to configure your project and then generate the complete binding structure for the chosen bindings.


## Building the Generated Bindings
Go to the respective generated folder and read the README.md file.

## Benefits of This Approach

1. **Single Source of Truth**: The Rosetta registration defines the API once; all bindings derive from it.

2. **Automatic Synchronization**: When you add a method to Rosetta, it appears in all language bindings automatically.

3. **Type Safety**: Generated TypeScript declarations and Python stubs enable IDE autocompletion and static analysis.

4. **Consistent API**: All language bindings follow the same structure, making it easy to port code between languages.

5. **Runtime Introspection**: Generated modules expose `list_classes()` and `get_class_methods()` for dynamic exploration.

## Extending the Generator
This is a work in progress to simplify your life!

## License

MIT

## Credits

[Xaliphostes](https://github.com/xaliphostes) (fmaerten@gmail.com)
