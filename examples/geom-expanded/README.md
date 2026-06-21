# geom-expanded — reflection-free bindings

A variant of [`../geom-lib`](../geom-lib) whose generated bindings **build with a
stock toolchain** — no clang-p2996, no reflection, (almost) no rosetta headers on
the machine that compiles the binding. It ships four such targets:

- **`python-expanded`** → a pybind11 module that builds with a stock **C++17** compiler.
- **`nanobind-expanded`** → a [nanobind](https://github.com/wjakob/nanobind) module
  (leaner/faster pybind11 successor) — stock **C++17**, smallest binary of the set.
- **`node-expanded`** → an N-API addon that builds with a stock **C++20** compiler
  (uses the header-only, reflection-free `node_runtime.h`).
- **`wasm-expanded`** → an emscripten/embind module that builds with a **stock emsdk**
  (no reflection-aware fork — the limitation noted for the plain `wasm` target).

Two things make that possible:

1. **Out-of-line annotations.** The headers in [`geom/`](geom) are plain C++:
   no `[[ = rosetta::doc{...} ]]`, no rosetta include. Every doc/range lives in a
   `*.ann.json` side-car ([`Point.ann.json`](Point.ann.json),
   [`Triangle.ann.json`](Triangle.ann.json), [`Model.ann.json`](Model.ann.json)),
   wired in by the manifest's `"annotations"` field. So the bound headers
   themselves are stock C++ and parse under any compiler.

2. **The `*-expanded` targets.** Instead of emitting a thin binding that re-runs
   the reflection walk at the target's compile time (the plain `python` /
   `nanobind` / `node` / `wasm` targets), these backends fully expand every field,
   method, constructor and enumerator into explicit pybind11 / nanobind / N-API /
   embind calls. The generated TU includes only the binding-framework headers plus
   the (stock) user headers.

The docstrings and the `range` validation on `Triangle::a/b/c` are baked into the
generated source as literal C++ — even though the headers carry none — because
reflection runs once on the **generation host**, not on the target. Members a
backend can't marshal (e.g. `Surface::transform`, which takes a `std::function`)
are skipped where unsupported; `std::vector` crosses the boundary in every target
(embind via emitted `register_vector<T>()`).

## Reproduce

### 1. scaffold the driver from the manifest (host: any C++ — rosetta_gen is plain)
```sh
../../bin/rosetta_gen manifest.json gen
```

### 2. build & run the driver (host: needs clang-p2996 — reflection)
```sh
cmake -S gen -B gen/build && cmake --build gen/build -j
./generator bindings
```

### 3a. Python / pybind11 — stock C++17
```sh
cmake -S bindings/python-expanded -B bindings/python-expanded/build
cmake --build bindings/python-expanded/build -j
```

### 3b. Python / nanobind — stock C++17 (needs the pip `nanobind` package)
```sh
cmake -S bindings/nanobind-expanded -B bindings/nanobind-expanded/build
cmake --build bindings/nanobind-expanded/build -j
```

### 3c. Node / N-API — stock C++20
```sh
( cd bindings/node-expanded && npm install && npm run build )
```

### 3d. WASM / embind — stock emsdk (no fork)
```sh
emcmake cmake -S bindings/wasm-expanded -B bindings/wasm-expanded/build
cmake --build bindings/wasm-expanded/build -j
#   -> bindings/wasm-expanded/build/geom.js + geom.wasm, loadable in node/web
```

## Run the examples

Each binding has a matching, self-contained script:

```sh
python3 example_pybind11.py   # python-expanded    (pygeom)
python3 example_nanobind.py   # nanobind-expanded  (nbgeom)

node    example_node.js       # node-expanded      (jsgeom — std::vector <-> JS Array)

node    example_wasm.js       # wasm-expanded      (geom — embind, async load + .delete())
python3 -m http.server 8000   # wasm-expanded running in a browser
```

The wasm module also runs in the **browser** — [`example_wasm.html`](example_wasm.html)
loads it, exercises the same API, prints the results and draws the surface on a
`<canvas>`. Serve over HTTP (browsers can't `fetch` a `.wasm` from `file://`):

```sh
python3 -m http.server 8000
# then open http://localhost:8000/example_wasm.html
```

All four exercise the same `Model` / `Surface` / `Point` / `Triangle` API. The two
Python scripts are identical apart from the build dir and module name (rosetta
exposes the same API from pybind11 and nanobind). `example_node.js` and
`example_wasm.js` differ only in how each backend marshals `std::vector` and
manages object lifetime (see the header comment in each file).

Step 2 is the only one that needs the C++26/reflection toolchain, and it runs on
*your* machine. Step 3 — what a downstream consumer actually compiles — is
ordinary pybind11 / nanobind / N-API / embind.
