# Dynamic Library Example

Shows how to bind a third party **dynamic (shared) library** using `Rosetta` for Python.

## Folder flower

The third party library that we cannot modify. A **dynamic library** (.so/.dylib/.dll) is generated. 

## Folder pyflower

The Rosetta introspection + python binding. Only the headers of the library `flower` are used, and the binding links against the dynamic library.

## Compilation 1 (step by step)

- Go to the **flower** folder, create a `build` directory and run `cmake .. && make`

- Go to the **pyflower** folder, create a `build` directory and run `cmake .. && make`

## Compilation 2 (all at once)

Create a `build` directory and run `cmake .. && make`

## Testing

In the folder **pyflower/build**, since the script `test.py` is copied, run:
```sh
python test.py
```

### Note on Dynamic Libraries

When using dynamic libraries, the shared library must be findable at runtime. The CMake configuration handles this by:

1. **Linux/macOS**: Setting the RPATH so the Python module knows where to find the .so/.dylib
2. **Windows**: Copying the .dll to the same directory as the .pyd file

If you encounter "library not found" errors, ensure the dynamic library is either:
- In the same directory as the Python module
- In a system library path (LD_LIBRARY_PATH on Linux, DYLD_LIBRARY_PATH on macOS, PATH on Windows)
- Or the RPATH is correctly set (handled by CMake in this example)