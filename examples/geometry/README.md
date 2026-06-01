# Geometry — manifest-driven bindings for a small C++ library

This example generates **Python** and **Node** bindings for a tiny geometry
library without touching its source. The classes in [`geom/`](geom) stay
pristine; everything else is produced by Rosetta from a single
[`manifest.json`](manifest.json).

## What it binds

[`geom/`](geom) is an ordinary, annotation-free C++ library:

| Class      | Shape                                                               |
| ---------- | ------------------------------------------------------------------- |
| `Point`    | `double x, y, z` — plus `Point(x, y, z)`                            |
| `Triangle` | `int a, b, c` — plus `Triangle(a, b, c)`                           |
| `Surface`  | holds `vector<Point>` + `vector<Triangle>`; built from flat arrays  |
| `Model`    | holds a `vector<Surface>`                                           |

Between them they exercise the interesting cases: **nested user types**
(`Surface` returns `Point`/`Triangle` objects), **`std::vector` members**,
and **multiple constructors** (default *and* parameterized). Members a
backend can't marshal — e.g. `Surface::transform(std::function<...>)` — are
skipped rather than failing the build.

## How it works

Generation is a two-stage pipeline:

```
manifest.json ──(rosetta_gen)──▶ gen/         ──(build)──▶ ./generator  ──(run)──▶ bindings/
                                 bindings.h                 (binary in this           python/ (module: pygeom)
                                 geometry.cpp                folder)                  node/   (addon:  jsgeom)
                                 CMakeLists.txt
```

1. **`rosetta_gen`** (built once into `<repo>/bin`) reads the manifest and
   writes a small project-specific *generator program* into the generation
   folder you name — `gen/` here.
2. Building that folder drops the compiled driver — the **`generator`**
   binary — directly into this folder (its parent), not the build tree.
3. Running `./generator --out bindings` uses C++26 reflection to walk each
   class and emit one **combined module per backend** under `bindings/`. All
   four classes land in a single module — `pygeom` for Python, `jsgeom` for
   Node — per the `targets` entries in the manifest.

The generation folder (`gen/`) must not share a name with the `generator`
binary, since both live here. Each backend's module name comes from its
`targets` entry.

## Building

⚠️ First, you need the `tools/rosetta_gen` to be compiled.

### Create the Generator for `geometry`

```bash
../../bin/rosetta_gen manifest.json gen
```

Compile it (the `generator` binary is dropped into the current folder):

```bash
cd gen
mkdir build && cd build
cmake .. && make
cd ../.. # root of geometry/
```

### Create the bindings

```bash
./generator --out bindings
```

### Compile for python

```bash
cd bindings/python
mkdir build && cd build
cmake .. && make
cd ../../../ # root geometry

python example.py
```

### Compile for node

```bash
cd bindings/node
npm i
npm run build
cd ../../ # root geometry

node example.js
```
