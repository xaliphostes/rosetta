# Geometry Binding Generator

Custom binding generator for the geometry project that properly populates the Rosetta registry before generating bindings.

## Why This Is Needed

The standard `binding_generator` executable doesn't know about your project's classes. It queries `rosetta::Registry::instance()` at generation time, but the registry is empty because your `register_rosetta_classes()` function was never called.

This custom generator solves the problem by:
1. Including your `registration.h`
2. Calling `register_rosetta_classes()` **before** generating
3. Now the generator finds all your classes (Point, Triangle, Surface, Model)

## Project Structure

Assuming your project looks like this:

```
your_project/
├── binding_generator/          # The binding generator source
│   ├── BindingGeneratorLib.h
│   ├── Pybind11Generator.h
│   ├── ... (other generator files)
│   └── CMakeLists.txt
├── include/
│   └── rosetta/
│       └── rosetta.h
├── third/                      # Your geometry headers
│   ├── common.h
│   ├── Model.h
│   ├── Point.h
│   ├── Surface.h
│   └── Triangle.h
├── bindings/
│   └── registration.h          # Your Rosetta registration
├── geometry_generator/         # THIS DIRECTORY
│   ├── my_generator.cxx
│   ├── CMakeLists.txt
│   └── README.md
└── project.json                # Your binding config
```

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

And the generated `generated_pybind11.cxx` will now contain:

```cpp
// --- Point ---
py::class_<Point, std::shared_ptr<Point>>(m, "Point")
    .def(py::init<>())
    .def(py::init<double, double, double>())
    .def_readwrite("x", &Point::x)
    .def_readwrite("y", &Point::y)
    .def_readwrite("z", &Point::z)
    ...

// --- Triangle ---
py::class_<Triangle, std::shared_ptr<Triangle>>(m, "Triangle")
    ...

// --- Surface ---
py::class_<Surface, std::shared_ptr<Surface>>(m, "Surface")
    ...

// --- Model ---
py::class_<Model, std::shared_ptr<Model>>(m, "Model")
    ...
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
