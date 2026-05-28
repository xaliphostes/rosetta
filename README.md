<p align="center">
  <img src="media/logo-rosetta.png" alt="rosetta" width="400">
</p>

# Rosetta (v2)

A C++26 reflection playground. One `consteval` walk over a class, plus a small
annotation surface, is enough to drive a growing set of backends — Python and
Node bindings, REST/QML/Qt glue, a Markdown doc generator, a moc-less
signals/slots/properties layer, and so on. The point is to keep the source
class clean and let reflection do the rest.


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
  annotations.h    shared annotation types: doc, range, readonly
  walk.h           single consteval walk<T>(visitor) over fields + methods,
                   forwarding each member with its full annotation pack
  docgen.h         walk visitor that emits a Markdown reference for T
  mini_moc.h       signals / slots / properties on top of reflection;
                   Qt-style ergonomics, no separate moc tool

tests/             one .cpp per concept; each builds to a runnable demo
examples/          bindings/ (python, node, rest, web), docgen/, qml/, qt/
docs/              design notes (FREE_FUNCTIONS, OTHER_ANNOTATIONS, TODO)
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

## A taste

### Annotated class + Markdown docs

```cpp
#include <rosetta/annotations.h>
#include <rosetta/docgen.h>
#include <iostream>

struct Person {
    [[= rosetta::doc{"the person's display name"}]]
    std::string name;

    [[= rosetta::doc{"age in whole years"}, = rosetta::range{0, 150}]]
    int age = 0;

    [[= rosetta::doc{"server-assigned identifier"}, = rosetta::readonly{}]]
    std::string id;

    [[= rosetta::doc{"Greet using the given salutation."}]]
    std::string greet(std::string const& salutation) const;
};

int main() {
    std::cout << rosetta::generate_markdown<Person>();
}
```

### Signals / slots / properties without moc

```cpp
#include <rosetta/mini_moc.h>
#include <string>

class Person {
public:
    [[= rosetta::moc::signal]] rosetta::moc::Signal<std::string const&> nameChanged;
    [[= rosetta::moc::signal]] rosetta::moc::Signal<int>                ageChanged;

    [[= rosetta::moc::property{"name", "nameChanged"}]] std::string m_name;
    [[= rosetta::moc::property{"age",  "ageChanged"}]]  int         m_age = 0;
};

struct Logger {
    [[= rosetta::moc::slot]] void onAge(int v) { /* ... */ }
};

int main() {
    Person p; Logger l;
    rosetta::moc::connect<"ageChanged", "onAge">(p, l);  // checked at compile time
    rosetta::moc::set<"age">(p, 30);                     // fires ageChanged
    rosetta::moc::set<"age">(p, 30);                     // equality-gated, silent
}
```

Wrong signal name → `static_assert`. Wrong argument types → normal template
error. No `.moc` file, no code-generation step.

## Examples

| Path                     | What it shows                                       |
|--------------------------|-----------------------------------------------------|
| `examples/docgen`        | Reflection-driven Markdown reference generator      |
| `examples/qt`            | Building a Qt widget form from a reflected struct   |
| `examples/qml`           | Exposing a reflected C++ object to QML              |
| `examples/bindings/python` | pybind11-style binding generated from reflection  |
| `examples/bindings/node` | N-API binding generated from reflection             |
| `examples/bindings/rest` | HTTP/REST surface generated from reflection         |
| `examples/bindings/web`  | WebAssembly binding (requires reflection-aware emsdk) |

## Design notes

- `docs/FREE_FUNCTIONS.md` — sketch for reflecting namespace-scope functions
- `docs/OTHER_ANNOTATIONS.md` — proposed annotation kinds beyond the current three
- `docs/TODO.md` — what the walker and visitor surface still miss (enums, bases,
  ctors, statics, parameter metadata, ...)

## License

[MIT](./LICENSE)

## Author
[xaliphostes](https://github.com/xaliphostes)