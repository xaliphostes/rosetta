# Rosetta Create

A project scaffolding tool for creating Rosetta binding projects.

## Installation

```bash
cd rosetta/tools
pip install .
```

Or install in development mode:

```bash
pip install -e .
```

## Usage

### Interactive Mode (Wizard)

Simply run without arguments to start the interactive wizard:

```bash
rosetta-create
```

The wizard will guide you through:
1. Project name and metadata
2. Library source (GitHub URL or local path)
3. Binding targets (Python, WASM, JavaScript, REST)
4. Output directory

### Command-Line Mode

For scripting or when you know what you want:

```bash
# Create project with GitHub library
rosetta-create \
  --name pmp-bindings \
  --lib-name pmp \
  --lib-repo https://github.com/pmp-library/pmp-library.git \
  --targets python,wasm \
  --output ./pmp-bindings

# Create project with local library
rosetta-create \
  --name mylib-bindings \
  --lib-name mylib \
  --lib-path ../mylib \
  --targets python
```

### Options

```
Project Information:
  --name, -n          Project name (required)
  --description, -d   Project description
  --author, -a        Author name
  --version, -v       Version (default: 1.0.0)
  --license, -l       License (default: MIT)

Library Source (one required):
  --lib-repo          Git repository URL for library
  --lib-path          Local path to library

Library Configuration:
  --lib-name          Name of the library to bind (required)
  --lib-tag           Git tag/branch (default: main)
  --lib-include-dir   Include subdirectory (default: include)
  --lib-src-dir       Source subdirectory (default: src)

Namespace:
  --namespace         C++ namespace (default: lib-name)
  --registration-namespace  Registration namespace

Targets:
  --targets, -t       Comma-separated: python,wasm,javascript,rest (default: python)

Output:
  --output, -o        Output directory (default: ./<name>)

Flags:
  --interactive, -i   Force interactive mode
  --force, -f         Overwrite existing directory
```

## Generated Project Structure

```
my-project/
├── CMakeLists.txt              # Build configuration
├── main.cxx                    # Generator entry point
├── project.json                # Binding configuration
├── README.md                   # Project documentation
├── .gitignore
└── bindings/
    └── my_project_registration.h   # Class/function registration
```

## Next Steps After Generation

1. Edit `bindings/*_registration.h` to register your C++ classes and functions
2. Build the generator: `mkdir build && cd build && cmake .. && make`
3. Run the generator: `./<name>_generator project.json`
4. Build/install the generated bindings

## Example

```bash
# Create a project for binding the Eigen library
rosetta-create \
  --name eigen-bindings \
  --lib-name eigen \
  --lib-repo https://gitlab.com/libeigen/eigen.git \
  --lib-tag 3.4.0 \
  --lib-include-dir "" \
  --lib-src-dir Eigen \
  --targets python \
  --output ./eigen-bindings
```
