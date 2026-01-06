# Geometry Binding Generator

Custom binding generator for the geometry project that properly populates the Rosetta registry before generating bindings.

## Why This Is Needed

The standard `binding_generator` executable doesn't know about the project's classes. It queries `rosetta::Registry::instance()` at generation time, but the registry is empty because the `register_rosetta_classes()` function is never called.

This custom generator solves the problem by:
1. Including the `registration.h` (i.e., your personal rosetta registration of a given C++ lib)
2. Calling `register_rosetta_classes()` **before** generating
3. Now the generator finds all the classes (Point, Triangle, Surface, Model)


## Build Instructions

### 1. Build geometry_generator binary

```bash
cd geometry_generator
mkdir build && cd build
cmake ..
make
```
This will create the binary `geometry_generator` in the `build`directory. 

### 2. Generate Bindings

After building:

```bash
# Still from the build directory
./geometry_generator ../project.json
```

#### Expected Output

After running, you should see:

```
Loading configuration: project.json
Configuration loaded:
  Project: geometry v1.0.0
  Output:  ./generated
  Targets: python, wasm, javascript, rest

✓ Python bindings: ./generated/python
✓ WASM bindings: ./generated/wasm
✓ JavaScript bindings: ./generated/javascript
✓ REST API server: ./generated/rest

✅ All bindings generated successfully!
```

All the generated targets are in the `build/generated` folder.


### 3. Compiling and testing
For each binding, go to the respective folder (`python`, `javascript`, `wasm`, `rest`...) and follow the instruction in the README. Typically, you will have to
- Python : 
    ```sh
    mkdir build && cd build
    cmake ..
    make
    ```
    or
    ```sh
    pip install .
    ```
- JavaScript: 
    ```sh
    npm i
    ```
- REST-API :
    ```sh
    mkdir build && cd build
    cmake ..
    make
    ```
- Wasm :
    ```sh
    mkdir build && cd build
    emcmake cmake ..
    emmake make
    ```