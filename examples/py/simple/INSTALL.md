# Compilation and Installation Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Installing Dependencies](#installing-dependencies)
3. [Project Structure](#project-structure)
4. [Compilation Methods](#compilation-methods)
5. [Testing](#testing)
6. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Software

- **C++ Compiler** with C++20 support:
  - GCC 10+ or Clang 12+ (Linux/macOS)
  - MSVC 2019+ (Windows)
- **CMake** 3.15 or later
- **Python** 3.6 or later
- **pip** (Python package manager)
- **Git** (for cloning dependencies)

### Check Your Environment

```bash
# Check C++ compiler
g++ --version          # or clang++ --version
# Should show version 10+

# Check CMake
cmake --version
# Should show version 3.15+

# Check Python
python3 --version      # or python --version
# Should show version 3.6+

# Check pip
pip3 --version         # or pip --version
```

---

## Installing Dependencies

### Ubuntu/Debian Linux

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install -y build-essential cmake git

# Install Python development headers
sudo apt install -y python3-dev python3-pip

# Install pybind11 (optional - CMake will fetch it if not found)
sudo apt install -y pybind11-dev

# Or install via pip
pip3 install pybind11
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake python

# Install pybind11 (optional)
brew install pybind11

# Or via pip
pip3 install pybind11
```

### Windows

#### Using Visual Studio

1. **Install Visual Studio 2019 or later** with:
   - Desktop development with C++
   - CMake tools for Windows
   - Python development workload

2. **Install Python** from [python.org](https://www.python.org/downloads/)
   - Make sure to check "Add Python to PATH"

3. **Install pybind11**:
```cmd
pip install pybind11
```

#### Using MSYS2/MinGW

```bash
# Update MSYS2
pacman -Syu

# Install toolchain
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-python

# Install pybind11
pip install pybind11
```

---

## Project Structure

Create your project directory:

```bash
mkdir rosetta_python_example
cd rosetta_python_example
```

---

## Compilation Methods

### Method 1: Using CMake (Recommended)

This is the most flexible method and works on all platforms.

#### Step 1: Create Build Directory

```bash
mkdir build
cd build
```

#### Step 2: Configure

```bash
# Linux/macOS
cmake ..

# Windows (Visual Studio)
cmake .. -G "Visual Studio 16 2019"

# Windows (MinGW)
cmake .. -G "MinGW Makefiles"
```

**Common CMake Options:**

```bash
# Specify Python version
cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.9

# Specify build type
cmake .. -DCMAKE_BUILD_TYPE=Release

# Specify installation prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Enable verbose output
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
```

#### Step 3: Build

```bash
# Linux/macOS
make -j$(nproc)         # Use all CPU cores

# Windows (Visual Studio)
cmake --build . --config Release

# Windows (MinGW)
mingw32-make
```

#### Step 4: Verify Build

After building, you should see:

```bash
build/
├── rosetta_example.so          # Linux
├── rosetta_example.dylib       # macOS
└── rosetta_example.pyd         # Windows
```

### Method 2: Using setup.py

This method uses Python's setuptools and is simpler for Python-only distribution.

#### Step 1: Install Build Dependencies

```bash
pip3 install pybind11 setuptools wheel
```

#### Step 2: Build in Development Mode

```bash
# Build and install in development mode (editable)
pip3 install -e .

# This creates a link to your source directory
# so changes to Python code are immediately available
```

#### Step 3: Build for Distribution

```bash
# Build wheel package
python3 setup.py bdist_wheel

# This creates:
# dist/rosetta_example-1.0.0-cp39-cp39-linux_x86_64.whl
```

#### Step 4: Install the Wheel

```bash
pip3 install dist/rosetta_example-*.whl
```

### Method 3: Direct Compilation (Manual)

For those who want full control:

#### Linux/macOS

```bash
# Get Python configuration
PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
PYTHON_LIB=$(python3 -c "from sysconfig import get_paths; print(get_paths()['stdlib'])")
PYTHON_EXT=$(python3-config --extension-suffix)

# Get pybind11 include path
PYBIND11_INCLUDE=$(python3 -c "import pybind11; print(pybind11.get_include())")

# Compile
g++ -O3 -Wall -shared -std=c++20 -fPIC \
    -I${PYTHON_INCLUDE} \
    -I${PYBIND11_INCLUDE} \
    -I. \
    binding_example.cxx \
    -o rosetta_example${PYTHON_EXT}
```

#### macOS Specifics

```bash
# macOS requires additional flags
g++ -O3 -Wall -shared -std=c++20 -fPIC \
    -undefined dynamic_lookup \
    -I${PYTHON_INCLUDE} \
    -I${PYBIND11_INCLUDE} \
    -I. \
    binding_example.cxx \
    -o rosetta_example${PYTHON_EXT}
```

#### Windows (MinGW)

```cmd
g++ -O3 -Wall -shared -std=c++20 ^
    -I%PYTHON_INCLUDE% ^
    -I%PYBIND11_INCLUDE% ^
    -I. ^
    binding_example.cxx ^
    -o rosetta_example.pyd ^
    -L%PYTHON_LIB% -lpython39
```

---

## Testing

### Method 1: Direct Python Test

```bash
# If using CMake, stay in build directory
cd build
python3 ../test_bindings.py

# If using setup.py, from project root
python3 test_bindings.py
```

### Method 2: Interactive Testing

```bash
python3
```

```python
>>> import rosetta_example as rosetta
>>> v = rosetta.Vector3D(3, 4, 12)
>>> print(v.length())
13.0
>>> v.normalize()
>>> print(f"x={v.x:.4f}, y={v.y:.4f}, z={v.z:.4f}")
x=0.2308, y=0.3077, z=0.9231
```

### Method 3: Using pytest

```bash
# Install pytest
pip3 install pytest

# Run tests
pytest test_bindings.py -v
```

---

## Complete Step-by-Step Example

Here's a complete walkthrough from scratch:

```bash
# 1. Create project directory
mkdir rosetta_python_example
cd rosetta_python_example

# 2. Copy files
# - Copy pybind11_generator.h
# - Copy binding_example.cxx
# - Copy test_bindings.py
# - Copy CMakeLists.txt
# - Copy setup.py
# - Copy Rosetta headers to rosetta/

# 3. Install dependencies (Ubuntu example)
sudo apt update
sudo apt install -y build-essential cmake python3-dev python3-pip
pip3 install pybind11

# 4. Build using CMake
mkdir build
cd build
cmake ..
make -j$(nproc)

# 5. Test
python3 ../test_bindings.py

# 6. (Optional) Install system-wide
sudo make install

# OR use setup.py method:
cd ..
pip3 install -e .
python3 test_bindings.py
```

---

## Troubleshooting

### Problem: "pybind11 not found"

**Solution:**
```bash
# Install pybind11
pip3 install pybind11

# Or specify path to CMake
cmake .. -Dpybind11_DIR=/path/to/pybind11
```

### Problem: "Python.h: No such file or directory"

**Solution:**
```bash
# Ubuntu/Debian
sudo apt install python3-dev

# macOS (usually included)
brew reinstall python

# Windows - reinstall Python with development headers
```

### Problem: "undefined symbol: _Py_..."

**Solution:**
```bash
# Make sure you're using the correct Python version
# Rebuild with explicit Python:
cmake .. -DPYTHON_EXECUTABLE=$(which python3)
```

### Problem: "C++20 features not supported"

**Solution:**
```bash
# Update compiler
# Ubuntu
sudo apt install g++-10
export CXX=g++-10

# Or use C++17 mode (edit CMakeLists.txt)
set(CMAKE_CXX_STANDARD 17)
```

### Problem: ImportError when testing

**Solution:**
```bash
# Check library is in correct location
ls build/*.so     # Linux
ls build/*.dylib  # macOS
ls build/*.pyd    # Windows

# Make sure you're in build directory or adjust Python path
export PYTHONPATH=$PYTHONPATH:$(pwd)/build
```

### Problem: "Symbol not found" on macOS

**Solution:**
```bash
# Use -undefined dynamic_lookup
# This is already in CMakeLists.txt, but for manual compilation:
g++ ... -undefined dynamic_lookup ...
```

### Problem: Windows DLL loading issues

**Solution:**
```cmd
# Make sure Python architecture matches build
python -c "import platform; print(platform.architecture())"

# Should match your compiler (x86 vs x64)
```

---

## Advanced Build Options

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### Optimized Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Build with Specific Compiler

```bash
# Use Clang
cmake .. -DCMAKE_CXX_COMPILER=clang++

# Use specific GCC version
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### Cross-compilation

```bash
# Example: Build for ARM on x86
cmake .. -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake
```

---

## Installing for System-wide Use

### Using CMake

```bash
cd build
sudo make install

# Default location: /usr/local/lib/python3.x/site-packages/
```

### Using pip

```bash
# Install from source directory
pip3 install .

# Or from wheel
pip3 install dist/rosetta_example-*.whl

# Uninstall
pip3 uninstall rosetta-example
```

---

## Performance Tips

1. **Use Release builds** for production:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

2. **Enable LTO** (Link Time Optimization):
   ```bash
   cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
   ```

3. **Use native CPU features**:
   ```bash
   cmake .. -DCMAKE_CXX_FLAGS="-march=native"
   ```

---

## Next Steps

After successful compilation:

1. ✅ Run test suite: `python3 test_bindings.py`
2. ✅ Try interactive Python session
3. ✅ Check example scripts
4. ✅ Read the README.md for usage examples
5. ✅ Start binding your own classes!

---

## Getting Help

If you encounter issues:

1. Check this troubleshooting section
2. Verify all prerequisites are installed
3. Try the verbose build: `cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON`
4. Check compiler error messages carefully
5. Ensure Rosetta headers are in correct location

For more help, see the project documentation or open an issue.