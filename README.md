<p align="center">
  <img src="media/logo-rosetta.png" alt="rosetta" width="400">
</p>

# Rosetta (v2)

A C++26 reflection playground that generates Python, Node, REST, and
WebAssembly bindings for **your existing classes — without modifying
them**. Point rosetta at a header via a small `manifest.json`, run one
tool, get per-language binding projects out.

Annotations (`doc`, `range`, `readonly`, …) are an *opt-in* enrichment,
not a requirement: add them where you want docstrings, validation, or
UI hints; leave the rest of the class alone. Reflection does the work
either way.


[![Slides](https://img.shields.io/badge/slides-rosetta-blue?logo=marp)](https://xaliphostes.github.io/rosetta2/)

## Status

Prototype. Tracks the in-flight C++26 reflection papers:

- **P2996** — reflection (`^^T`, `[: r :]` splice, `std::meta::*`)
- **P3394** — annotation attributes (`[[= rosetta::doc{"..."}]]`)
- **P3294** — token injection (not yet used; see notes in `mini_moc.h`)

Builds with the Bloomberg [clang-p2996 fork](https://github.com/bloomberg/clang-p2996).
No mainline compiler implements enough of these proposals yet.

## Requirements

- A clang-p2996 build at `$HOME/devs/c++/clang-p2996/build` (or override
  `CLANG_P2996_ROOT` when invoking cmake).
- CMake 3.28+, Ninja or Make.
- C++26 mode with the fork's flags: `-freflection -freflection-latest
  -fexperimental-library`. Annotation-using code also needs
  `-fannotation-attributes`.

## Layout

```
include/rosetta/
  annotations.h    shared annotation types: doc, range, readonly, ...
  walk.h           single consteval walk<T>(visitor) over fields + methods,
                   forwarding each member with its full annotation pack
  docgen.h         walk visitor that emits a Markdown reference for T
  generate.h       reflection-driven binding scaffolder: rosetta::generate<T>
  mini_moc.h       signals / slots / properties on top of reflection;
                   Qt-style ergonomics, no separate moc tool
  visitors/        per-backend bind kits (pybind, N-API, REST, emscripten)

tools/
  rosetta_gen/     framework tool: manifest.json → project-specific scaffolder

tests/             one .cpp per concept; each builds to a runnable demo
examples/
  bindings/        hand-written reference backends (python, node, rest, web)
  generate/        manifest-driven, generated bindings (no class modification)
  docgen/  qml/  qt/  moc/
docs/              QUICKSTART, GENERATE, design notes (FREE_FUNCTIONS, ...)
```

## Build the test suite

```bash
cd tests
cmake -B build
cmake --build build
./build/hello
./build/moc
./build/docgen
# ...
```

Each test is self-contained; pick by name (see `tests/CMakeLists.txt`).

## A taste — bind your existing library

You have this header. Don't change it:

```cpp
// my_lib/person.h
#include <string>

struct Person {
    std::string name;
    int         age = 0;
    std::string id;
    std::string greet(const std::string &salutation) const;
};
```

Write a small `manifest.json` somewhere alongside it:

```json
{
  "user_include": "../my_lib",
  "rosetta_include": "/path/to/rosetta/include",
  "classes": [
    { "name": "Person", "header": "person.h",
      "lib": "my_person", "targets": ["python", "node", "rest", "web"] }
  ]
}
```

Run the framework tool, build the project-specific tool it emits, run
that:

```bash
# (one-time) build the framework scaffolder
cmake -G Ninja -S tools/rosetta_gen -B tools/rosetta_gen/build
cmake --build tools/rosetta_gen/build

# manifest.json → bindings.h + my_person_gen.cpp + CMakeLists.txt
./tools/rosetta_gen/build/rosetta_gen path/to/manifest.json

# build & run the generated tool → per-backend project tree under output/
cmake -G Ninja -S path/to/generated -B path/to/generated/build
cmake --build path/to/generated/build
./path/to/generated/build/my_person_gen --out path/to/output
```

Result: `output/{python,node,rest,web}/` — each a self-contained CMake
project. `cd output/python && cmake -B build && cmake --build build`
and you `import my_person`.

The full walkthrough is in [`docs/QUICKSTART.md`](./docs/QUICKSTART.md);
the manifest schema, the `binding_info<T>` trait, and the layered
tooling model are in [`docs/GENERATE.md`](./docs/GENERATE.md). The
worked example lives in `examples/generate/`.

## Examples

| Path                       | What it shows                                       |
|----------------------------|-----------------------------------------------------|
| `examples/generate`        | Manifest-driven generation (no class modification)  |
| `examples/docgen`          | Reflection-driven Markdown reference generator      |
| `examples/qt`              | Building a Qt widget form from a reflected struct   |
| `examples/qml`             | Exposing a reflected C++ object to QML              |
| `examples/bindings/python` | Hand-written pybind11 backend (reference)           |
| `examples/bindings/node`   | Hand-written N-API backend (reference)              |
| `examples/bindings/rest`   | Hand-written HTTP/REST backend (reference)          |
| `examples/bindings/web`    | Hand-written WebAssembly backend (requires reflection-aware emsdk) |

## Design notes

- `docs/QUICKSTART.md` — five-step guide to generating bindings for an existing library
- `docs/GENERATE.md` — full reference for `rosetta::generate`, the manifest schema, and the tool layering
- `docs/FREE_FUNCTIONS.md` — sketch for reflecting namespace-scope functions
- `docs/OTHER_ANNOTATIONS.md` — proposed annotation kinds beyond the current three
- `docs/TODO.md` — what the walker and visitor surface still miss (enums, bases,
  ctors, statics, parameter metadata, ...)

## License

[MIT](./LICENSE)

## Author
[xaliphostes](https://github.com/xaliphostes)