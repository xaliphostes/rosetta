# geom-expanded — reflection-free bindings

A variant of [`../geom-lib`](../geom-lib) whose generated bindings **build with a
stock toolchain** — no clang-p2996, no reflection, no rosetta headers on the
machine that compiles the binding. It ships two such targets:

- **`python-expanded`** → a pybind11 module that builds with a stock C++17 compiler.
- **`wasm-expanded`** → an emscripten/embind module that builds with a **stock emsdk**
  (no reflection-aware fork — the limitation noted for the plain `wasm` target).

Two things make that possible:

1. **Out-of-line annotations.** The headers in [`geom/`](geom) are plain C++17:
   no `[[ = rosetta::doc{...} ]]`, no rosetta include. Every doc/range lives in a
   `*.ann.json` side-car ([`Point.ann.json`](Point.ann.json),
   [`Triangle.ann.json`](Triangle.ann.json), [`Model.ann.json`](Model.ann.json)),
   wired in by the manifest's `"annotations"` field. So the bound headers
   themselves are stock C++ and parse under any compiler.

2. **The `*-expanded` targets.** Instead of emitting a thin binding that re-runs
   the reflection walk at the target's compile time (the plain `python` / `wasm`
   targets), these backends fully expand every field, method, constructor and
   enumerator into explicit pybind11 / embind calls. The generated TU includes
   only the binding framework headers plus the (stock) user headers.

The docstrings and the `range` validation on `Triangle::a/b/c` are baked into the
generated source as literal C++ — even though the headers carry none — because
reflection runs once on the **generation host**, not on the target. Members embind
can't marshal (e.g. `Surface::transform`, which takes a `std::function`) are
skipped; `std::vector` members work via emitted `register_vector<T>()`.

## Reproduce

```sh
# 1. scaffold the driver from the manifest  (host: any C++ — rosetta_gen is plain)
../../bin/rosetta_gen manifest.json gen

# 2. build & run the driver                 (host: needs clang-p2996 — reflection)
cmake -S gen -B gen/build && cmake --build gen/build -j
./generator bindings

# 3a. Python: build with a STOCK C++17 compiler + pybind11
cmake -S bindings/python-expanded -B bindings/python-expanded/build
cmake --build bindings/python-expanded/build -j
( cd bindings/python-expanded && python3 -c "import pygeom; print(pygeom.Point.x.__doc__)" )
#   -> The x-coordinate of the point        (doc came from Point.ann.json)

# 3b. WASM: build with a STOCK emsdk (no fork)
emcmake cmake -S bindings/wasm-expanded -B bindings/wasm-expanded/build
cmake --build bindings/wasm-expanded/build -j
#   -> bindings/wasm-expanded/build/geom.js + geom.wasm, loadable in node/web

# 3c. Node: build the N-API addon with a stock C++ compiler
( cd bindings/node-expanded && npm install && npm run build )
```

## Run the examples

Each binding has a matching, self-contained script:

```sh
python3 example.py        # python-expanded  (pygeom)
node    example_node.js   # node-expanded    (jsgeom — std::vector <-> JS Array)
node    example_wasm.js   # wasm-expanded    (geom — embind, async load + .delete())
```

`example_node.js` and `example_wasm.js` exercise the same `Model`/`Surface`/
`Point`/`Triangle` API; they differ only in how each backend marshals
`std::vector` and manages object lifetime (see the header comment in each file).

Step 2 is the only one that needs the C++26/reflection toolchain, and it runs on
*your* machine. Step 3 — what a downstream consumer actually compiles — is
ordinary pybind11 / embind.
