# Geometry Binding Generator

Custom binding generator for the geometry project that properly populates the Rosetta registry before generating bindings.

## Why This Is Needed

The standard `binding_generator` executable doesn't know about the project's classes. It queries `rosetta::Registry::instance()` at generation time, but the registry is empty because the `register_rosetta_classes()` function is never called.

This custom generator solves the problem by:
1. Including the `registration.h`
2. Calling `register_rosetta_classes()` **before** generating
3. Now the generator finds all the classes (Point, Triangle, Surface, Model)


## Build Instructions

### Option 1: Build from geometry_generator directory

```bash
cd geometry_generator
mkdir build && cd build

cmake .. \
    -DBINDING_GENERATOR_DIR=../../binding_generator \
    -DROSETTA_INCLUDE_DIR=../../include \
    -DGEOMETRY_PROJECT_DIR=../..

make
```

### Option 2: Specify absolute paths

```bash
cmake .. \
    -DBINDING_GENERATOR_DIR=/path/to/binding_generator \
    -DROSETTA_INCLUDE_DIR=/path/to/rosetta/include \
    -DGEOMETRY_PROJECT_DIR=/path/to/geometry_project

make
```

## Generate Bindings

After building:

```bash
# From the build directory
./geometry_binding_generator ../../project.json

# Or from project root
./geometry_generator/build/geometry_binding_generator project.json
```

Or use the convenience target:

```bash
make generate_bindings
```

## Expected Output

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

✓ All bindings generated successfully!
```

## Troubleshooting

### "rosetta/rosetta.h not found"
Make sure `ROSETTA_INCLUDE_DIR` points to the directory containing `rosetta/rosetta.h`.

### "registration.h not found"
Make sure `GEOMETRY_PROJECT_DIR` points to your project root that contains `bindings/registration.h`.

### "BindingGeneratorLib.h not found"
Make sure `BINDING_GENERATOR_DIR` points to the directory containing all the generator headers.

### Generated bindings still empty
Verify that `register_rosetta_classes()` is being called by adding a print statement:
```cpp
void register_rosetta_classes() {
    std::cout << "Registering classes with Rosetta...\n";
    // ... rest of registration
}
```

## Compiling and testing
For each binding, go to the respective folder
