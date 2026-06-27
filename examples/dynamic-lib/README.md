# dynamic-lib — binding a separately-compiled C++ shared library

Every other example binds a **header-only** library: the method bodies live in
the same headers rosetta reflects over, so the generated binding just `#include`s
them and needs nothing at link time. Real libraries aren't shaped that way — they
ship *declarations* in `include/` and *definitions* compiled into a shared object.

This example binds exactly that shape. It has two halves:

```
dynamic-lib/
├── space/                     a pure, stock-C++ library — NO rosetta, NO annotations
│   ├── include/               PUBLIC API: declarations only (Vector3.h, BoundingBox.h)
│   ├── src/                   the bodies (Vector3.cpp, BoundingBox.cpp)
│   ├── bin/                   -> libspace.dylib / .so  (build output)
│   └── CMakeLists.txt         builds the SHARED library
│
└── rosetta/                   the binding side
    ├── manifest.json          points at ../space, links against libspace (see "user_lib")
    ├── Vector3.ann.json       out-of-line docs (the space headers stay pristine)
    ├── BoundingBox.ann.json
    ├── example_python.py
    └── example_node.js
```

`space` is an ordinary library: `space/include/*.h` only **declare** `Vector3` /
`BoundingBox`; the bodies are in `space/src/*.cpp` and compiled into
`space/bin/libspace.*`. The headers carry no rosetta include and no annotations —
they build with any stock compiler and sit in a real `namespace space`.

## The new manifest capability: `user_lib`

For a split library the generated binding must **link against** the shared object
to reach the out-of-line definitions. The manifest grew a field for that:

```json
"user_lib": {
    "name": "space",          // the library base name  (-lspace, libspace.dylib)
    "dir":  "../space/bin"     // where it was built     (relative to the manifest)
}
```

When set, rosetta emits the link wiring into the generated CMake projects:

```cmake
target_link_directories(<target> PRIVATE <dir>)
target_link_libraries(<target>   PRIVATE <name>)
set_target_properties(<target> PROPERTIES BUILD_RPATH "<dir>" INSTALL_RPATH "<dir>")
```

— for each C++ binding backend (the reflection `python` / `node` / … projects and
their `*-expanded` variants) **and** for the generator driver itself (the
reflection walk instantiates each bound type, so it needs the definitions at link
time too). Without `user_lib` the block is skipped and the project stays
header-only, exactly as before. See the field documented in
[`tools/rosetta_gen/rosetta_gen.cpp`](../../tools/rosetta_gen/rosetta_gen.cpp) and
plumbed through [`include/rosetta/generate.h`](../../include/rosetta/generate.h).

### Namespaces

`space::Vector3` lives in a namespace, but rosetta's C++ backends spell bound
types by their *unqualified* identifier (`bind_pybind<Vector3>`,
`py::class_<Vector3>`, …). So the generator derives each type's enclosing
namespace by reflection and emits a `using namespace space;` into each generated
TU — automatic, nothing to declare. It lands right after the user headers, ahead
of the embedded annotation specializations and the binding code that all spell
the type unqualified:

```cpp
// auto_pybind.cpp (generated)
#include "Vector3.h"
using namespace space;            // <- emitted from reflection
PYBIND11_MODULE(space, m) { rosetta::bind_pybind<Vector3>(m, "Vector3"); ... }
```

## Reproduce

The one-shot script runs the whole pipeline (see the per-step commands inside):

```sh
./build.sh
```

Or by hand, from this directory:

```sh
# 1. the pure-C++ shared library (stock compiler — no rosetta)
cmake -S space -B space/build && cmake --build space/build -j     # -> space/bin/libspace.*

# 2. scaffold + build the generator driver (needs clang-p2996)
( cd rosetta && ../../bin/rosetta_gen manifest.json gen )
cmake -S rosetta/gen -B rosetta/gen/build && cmake --build rosetta/gen/build -j

# 3. run it -> rosetta/bindings/{python,node,wasm-expanded}
( cd rosetta && ./generator bindings )

# 4a. python (reflection binding; CMake selects clang-p2996), linked to libspace
cmake -S rosetta/bindings/python -B rosetta/bindings/python/build
cmake --build rosetta/bindings/python/build -j

# 4b. node (reflection binding; CMake selects clang-p2996), linked to libspace
( cd rosetta/bindings/node && npm install && npm run build )
```

Then run the examples (the bindings' rpath points at `space/bin`, so
`libspace.*` is found wherever you run from):

```sh
cd rosetta
python3 example_python.py
node    example_node.js
```

```
Vector3(3, 0, 4).length() = 5.0          # body resolved from libspace.dylib
a.cross(b) = (-3.0, 6.0, -3.0)
box.center()   = (1.0, 1.0, 1.0)
doc(Vector3.x) = The x-coordinate (first Cartesian axis)   # out-of-line annotation
```

## WebAssembly

`wasm-expanded` is generated with the same `user_lib` wiring, but a native
`.dylib`/`.so` **cannot** be linked into WebAssembly. So `space` is built a
second time as a *wasm static archive* with the **same emsdk** — its
`CMakeLists.txt` emits a `STATIC` library under `EMSCRIPTEN`, producing
`space/bin/libspace.a` right next to the native `libspace.dylib`. The two
coexist; each toolchain's linker picks the form it can use, so `user_lib.dir`
already points at the right place (`space/bin`) — nothing to repoint.

`build.sh` runs this only when `emcmake` is on `PATH`:

```sh
# build space as a wasm static archive (libspace.a) with the same emsdk
emcmake cmake -S space -B space/build-wasm && cmake --build space/build-wasm
# then the embind module — links libspace.a statically into space.wasm
emcmake cmake -S rosetta/bindings/wasm-expanded -B rosetta/bindings/wasm-expanded/build
cmake --build rosetta/bindings/wasm-expanded/build

cd rosetta && node example_wasm.js
```

```
Vector3(3, 0, 4).length() = 5            # body resolved from libspace.a (baked into space.wasm)
a.cross(b) = [ -3, 6, -3 ]
box.center()   = [ 1, 1, 1 ]
```

Because the wasm module target is itself named `space` (the manifest target),
the generated link line uses an explicit `-l` flag —
`target_link_libraries(space PRIVATE "-l${ROSETTA_USER_LIB}")` — so CMake links
the external archive by file name instead of mistaking it for the same-named
module target.
