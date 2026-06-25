<p align="center">
  <img src="media/logo-rosetta.png" alt="rosetta" width="400">
</p>

# Rosetta

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-26-blue.svg?logo=cplusplus" alt="C++26">
  <img src="https://img.shields.io/badge/status-prototype-yellow.svg" alt="Status: prototype">
  <a href="https://xaliphostes.github.io/rosetta/#1"><img src="https://img.shields.io/badge/slides-rosetta-blue?logo=marp" alt="Slides"></a>
  <a href="https://github.com/xaliphostes/rosetta2/stargazers"><img src="https://img.shields.io/github/stars/xaliphostes/rosetta2?style=social" alt="GitHub stars"></a>
</p>

<p align="center">
  <a href="https://github.com/bloomberg/clang-p2996"><img src="https://img.shields.io/badge/clang--p2996-tested-brightgreen.svg?logo=llvm" alt="clang-p2996: tested"></a>
  <img src="https://img.shields.io/badge/EDG-experimental-yellow.svg" alt="EDG: experimental">
  <img src="https://img.shields.io/badge/NVC%2B%2B-planned-lightgrey.svg?logo=nvidia" alt="NVC++: planned">
  <img src="https://img.shields.io/badge/GCC-in%20progress-lightgrey.svg?logo=gnu" alt="GCC: in progress">
  <img src="https://img.shields.io/badge/Clang%20%7C%20MSVC-tracking-lightgrey.svg" alt="Clang | MSVC: tracking">
</p>

A C++26 reflection playground that generates Python (pybind11 / nanobind), Node, WebAssembly, Qt, QML, REST, Julia, OpenAPI, JSON, TypeScript, C#, Markdown, HTML, ParaView... bindings for **your existing classes — without modifying them**. Point rosetta at a header via a small `manifest.json`, run one tool, get per-language binding projects out.

> **Your target compiler doesn't support reflection?** Generate the expanded binding once on a Linux or macOS host with a C++26 / P2996 compiler — e.g. the [Bloomberg `clang-p2996`](https://github.com/bloomberg/clang-p2996) fork — then ship and build the generated sources anywhere with a stock toolchain (plain Clang / GCC / MSVC, or a stock emsdk for WebAssembly). No reflection is needed on the target (see the **expanded** backends below).

Annotations (`doc`, `range`, `readonly`, …) are an *opt-in* enrichment, not a requirement: add them where you want docstrings, validation, or UI hints; leave the rest of the class alone. Reflection does the work either way.

## Features

Everything below is discovered by **reflection** from your unmodified headers — you declare *what* to bind in `manifest.json`, never *how*.

**What rosetta can bind**

- **Public fields** — exposed as read/write properties, with per-backend getters/setters.
- **Public methods** — both instance and `static` members.
- **Inheritance** — public base-class fields and methods are flattened into the derived binding; a derived declaration shadows the base one (most-derived wins) and a virtual diamond collapses to a single member. Virtual / overriding methods are flagged (`virtual_spec`) so backends can tell them apart from plain ones.
- **Multiple constructors** — default *and* parameterized; each overload is bound.
- **Enums** — `enum` / `enum class`, with enumerators surfaced as named constants.
- **Free (non-member) functions** — declared in the manifest, no edit to your headers ([details](docs/FREE_FUNCTIONS.md)).
- **Nested user types & `std::vector`** — `Surface` returning `Point`/`Triangle`, vector members, etc. are marshalled across the language boundary.
- Members a backend can't marshal (e.g. `std::function` params) are **skipped**, not fatal.

**Opt-in annotations** (enrich without intruding — see [annotations](docs/OTHER_ANNOTATIONS.md))

- `doc{"..."}` — docstrings / generated reference text.
- `readonly` — read-only property (write is rejected per backend).
- `range{lo, hi}` — value-range validation on assignment.
- `combobox{{...}}` — enumerated choices (UI hint).

Don't want to touch the header at all? Provide the same annotations **out of line**, from a JSON string attached to the type elsewhere — even in another file:

```cpp
struct MyClass {
    int value = 0;
};

// Add annotation later on in another file
template <>
constexpr std::string_view rosetta::ann_json_source<MyClass> = 
  R"({ "value": { "range": [1, 9] } })";
```

In a manifest-driven build you don't write that by hand: add an `"annotations": "Type.ann.json"` field to the class in `manifest.json` and rosetta bakes the external file in for you. Either way the metadata is merged with any inline annotations and reaches every backend (Python, Node, REST, OpenAPI, …) — see [out-of-line annotations](docs/OUT_OF_LINE_ANNOTATIONS.md).

**Backends** (one combined module per target, from a single generator)

| # | Target | Output | C++26 status |
|---|---|---|---|
| 1 | **Python** | pybind11 extension module | ✅ Working |
| 2 | **Python (expanded)** | `python-expanded` — fully-expanded pybind11 | ✅ Builds with a **stock C++17** compiler — no reflection on the target |
| 3 | **Python (nanobind)** | `nanobind` extension module — leaner/faster pybind11 successor | ✅ Working (≈½ the binary size of the pybind11 build) |
| 4 | **Python (nanobind, expanded)** | `nanobind-expanded` — fully-expanded nanobind | ✅ Builds with a **stock C++17** compiler — no reflection on the target |
| 5 | **Node** | N-API native addon | ✅ Working |
| 6 | **Node (expanded)** | `node-expanded` — fully-expanded N-API | ✅ Builds with a **stock C++20** compiler — no reflection on the target |
| 7 | **Julia** | CxxWrap.jl / jlcxx shared module | ✅ Builds & runs <br> ⚠️ `std::vector` skipped (fork libc++ gap) |
| 8 | **WebAssembly** | Emscripten/embind module | ⚠️ Needs a reflection-aware emsdk |
| 9 | **WebAssembly (expanded)** | `wasm-expanded` — fully-expanded embind | ✅ Builds with a **stock emsdk** (`std::vector` via `register_vector`) |
| 10 | **Qt Widgets** | live property/method inspector window (`QtVisitor`) | ✅ Working (needs reflection at the inspector's compile time) |
| 11 | **Qt Widgets (expanded)** | `qt-expanded` — generated inspector via `qt_widgets_runtime.h` | ✅ Builds with a **stock C++17** compiler + Qt 6 (no moc on generated code) |
| 12 | **QML** | QtQuick inspector via a generic `ReflectedObject` (`QmlVisitor`) | ✅ Working (needs reflection at compile time) |
| 13 | **QML (expanded)** | `qml-expanded` — fills the generic `ReflectedObject` explicitly | ✅ Builds with a **stock C++17** compiler + Qt 6 (moc only on the generic bridge) |
| 14 | **REST** | cpp-httplib JSON server (CRUD + method routes) + a generated `index.html` browser client, with `/openapi.json` and Swagger UI at `/docs` | ✅ Working |
| 15 | **OpenAPI** | OpenAPI 3.1 spec describing the REST surface — annotations become schema constraints (`range`→min/max, `readonly`→readOnly, `combobox`→enum) | ✅ Working |
| 16 | **JSON** | reflection-based nlohmann (de)serialization — one reusable `json_visitor.h`, no per-type code | ✅ Working |
| 17 | **TypeScript** | ambient `.d.ts` type declarations | ✅ Working |
| 18 | **Markdown** | API reference document | ✅ Working |
| 19 | **HTML** | self-contained, styled API reference page (anchored TOC, field/enum tables; annotations become description tags) | ✅ Working |
| 20 | **ParaView** | Server Manager XML for a plugin: fields → properties with range / **enumeration** / boolean / string-list domains, **default values** (from member initializers), `readonly`→`information_only`, plus a pipeline **`InputProperty`** and **`ArrayListDomain`** array-selection. Proxy `class=`/group/input from `paraview_proxy` / `paraview_input` / `paraview_array` annotations | ✅ Working (single input port) |
| 21 | **C#** | `csharp` — a native shared library exposing a flat C ABI (`rosetta_csharp_*`) plus idiomatic handle-backed C# wrapper classes that reach it through P/Invoke (values marshalled as JSON with `System.Text.Json`); ships a `.csproj`. `readonly`→get-only property, `range`→bounds-checked setter | ✅ Working |
| 22 | **C# (expanded)** | `csharp-expanded` — same C# wrapper / `.csproj`, but the native shim registers every member by pointer (no reflection walk) | ✅ Builds with a **stock C++20** compiler — no reflection on the target |

> New backends register without touching the generator, thanks to the visitor pattern — see [EXTENDING_BACKEND](docs/EXTENDING_BACKEND.md).

**Expanded (reflection-free) targets.** The default `python` / `nanobind` / `node` / `wasm` / `qt` / `qml` / `csharp` backends emit a *thin* binding that re-runs the reflection walk at the target's compile time, so building the binding also needs the C++26 toolchain. The `python-expanded`, `nanobind-expanded`, `node-expanded`, `wasm-expanded`, `qt-expanded`, `qml-expanded` and `csharp-expanded` targets instead **fully expand** every field, method, constructor and enumerator into explicit pybind11 / nanobind / N-API / embind / Qt / member-pointer calls. Reflection runs once, on the generation host; the generated binding is ordinary C++ that builds with a stock compiler — a plain C++17/20 compiler, a stock emsdk, or stock Qt 6 (the host still needs C++26 to *run the generator*, the target does not). This pairs naturally with [out-of-line annotations](docs/OUT_OF_LINE_ANNOTATIONS.md) so the bound headers stay stock C++ too — see [`examples/geom-expanded`](examples/geom-expanded).

## Mini-MOC — Qt signals / slots / properties, without moc

Beyond binding generation, rosetta ships [`mini_moc.h`](include/rosetta/mini_moc.h): a header-only, **moc-less** reimagining of Qt's signals/slots/properties built directly on C++26 reflection (P2996) + annotations (P3394). No code generator, no separate compile step — just annotate members and connect them.

You mark members with annotations and reach them through three free functions:

```cpp
#include <rosetta/mini_moc.h>
namespace moc = rosetta::moc;

class Person {
public:
    moc::Signal<std::string const &> nameChanged;   // a Signal<...> member IS a signal
    moc::Signal<int>                 ageChanged;

    [[= moc::property{"name", "nameChanged"}]] std::string m_name;
    [[= moc::property{"age",  "ageChanged"}]]  int         m_age = 0;
    [[= moc::property{"id"}]]                  int         m_id  = 0;   // no NOTIFY
};

struct Logger {
    [[= moc::slot]] void onAge(int v)               { total += v; }
    [[= moc::slot]] void onName(std::string const&) { /* ... */ }
    int total = 0;
};

Person p; Logger l;
moc::connect<"ageChanged", "onAge">(p, l);  // compile-time checked
moc::set<"age">(p, 30);                      // equality-gated; fires NOTIFY -> Logger::onAge
moc::get<"age">(p);                          // -> 30
```

- **Signals need no annotation** — any `Signal<...>` data member is a signal, recognized by its type. `slot` and `property{"name", "notifySig"}` annotations mark the members that *aren't* self-identifying; reflection discovers them.
- **`connect<"sig","slot">(sender, receiver)`** — compile-time checked: a wrong name is a `static_assert`, mismatched argument types are a template error.
- **`get<"prop">` / `set<"prop">`** — property access from outside the class (token injection, P3294, isn't in clang-p2996 yet, so accessors aren't emitted into the class body). `set<>` is equality-gated and fires the property's `NOTIFY` signal only on an actual change.
- **`Signal<Args...>`** — the only machinery type you spell out; supports `connect` / `disconnect` / `disconnect_all`, re-entrant self-disconnect, and a `ScopedConnection` RAII handle for scope-bound connections.

See the [`examples/moc`](examples/moc) demo and the test suite in [`tests/moc.cpp`](tests/moc.cpp).

## Status

Prototype. Tracks the in-flight C++26 reflection papers:

- **P2996** — reflection (`^^T`, `[: r :]` splice, `std::meta::*`)
- **P3394** — annotation attributes (`[[= rosetta::doc{"..."}]]`)
- **P3294** — token injection (not yet used; see notes in `mini_moc.h`)

Builds with the Bloomberg [clang-p2996 fork](https://github.com/bloomberg/clang-p2996) — the reference implementation rosetta is developed and tested against.

No mainline compiler ships these proposals yet, but other front-ends are implementing P2996 and should become viable targets as their support matures (and as rosetta's compiler-specific flags are abstracted):

- **clang-p2996** (Bloomberg fork) — ✅ supported today; what rosetta is built and tested with.
- **EDG** — front-end has an experimental P2996 implementation; the most likely next target.
- **NVC++ / NVHPC** — built on the EDG front-end, so it could inherit reflection as EDG's support lands in releases.
- **GCC** — reflection is under active development on experimental branches; not yet usable for rosetta.
- **Mainline Clang / MSVC** — tracking P2996 but no usable implementation yet.

Annotations (P3394) and token injection (P3294) are newer and currently exist only in the clang-p2996 fork, so full-feature builds remain fork-only for now.

## Requirements

- A clang-p2996 build at `$HOME/devs/c++/clang-p2996/build` (or override `CLANG_P2996_ROOT` when invoking cmake).
- CMake 3.28+, Ninja or Make.
- C++26 mode with the fork's flags: `-freflection -freflection-latest -fexperimental-library`. Annotation-using code also needs `-fannotation-attributes`.


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

Write a small `manifest.json` next to it. Each `targets` entry names the module/library produced for that backend; list every class you want bound:

```json
{
  "user_include": "../my_lib",
  "rosetta_include": "/path/to/rosetta/include",
  "targets": [
    { "lang": "python", "name": "person_py" },
    { "lang": "node",   "name": "person_js" }
  ],
  "classes": [
    { "name": "Person", "header": "person.h" }
  ]
}
```

Build the scaffolder once, then from your project folder generate, build, and run the project-specific tool it emits:

```bash
# (one-time) build the framework scaffolder → <repo>/bin/rosetta_gen
cmake -S tools/rosetta_gen -B tools/rosetta_gen/build
cmake --build tools/rosetta_gen/build

# from the folder holding manifest.json:
#   write the generator project (bindings.h + <generator_name>.cpp + CMakeLists.txt)
#   into a folder you name — here `gen/`
/path/to/rosetta/bin/rosetta_gen manifest.json gen

# build it — the `generator` binary is dropped into the current folder,
# not the build tree
cmake -S gen -B gen/build && cmake --build gen/build

# run it → one combined module per backend under bindings/
./generator bindings
```

Result: `bindings/{python,node}/` — each a self-contained CMake project exposing **all** your classes in a single module. `cd bindings/python && cmake -B build && cmake --build build`, then `import person_py`.

> `generator_name` and `module_name` are optional manifest fields: `generator_name` (the generated `.cpp` / usage name) defaults to the manifest's folder name, and a bare-string target like `"node"` falls back to `module_name` for its module name.

The full walkthrough is in [`docs/QUICKSTART.md`](./docs/QUICKSTART.md); the manifest schema, the `binding_info<T>` trait, and the layered tooling model are in [`docs/GENERATE.md`](./docs/GENERATE.md). The worked examples live in `examples/manifest/` and `examples/geom-lib/`.

## Examples

| Path                       | What it shows                                       |
|----------------------------|-----------------------------------------------------|
| `examples/manifest`        | Manifest-driven generation for `Person` (no class modification) |
| `examples/annotate-manifest`| Out-of-line annotations from an external JSON file, wired by the manifest's `annotations` field ([details](docs/OUT_OF_LINE_ANNOTATIONS.md)) |
| `examples/geom-lib`        | Manifest-driven bindings for a small geometry library (nested types, vectors) |
| `examples/geom-expanded`   | Reflection-free `python-expanded` / `nanobind-expanded` / `node-expanded` / `wasm-expanded` / `qt-expanded` / `qml-expanded` bindings (stock compiler, stock emsdk, stock Qt) with out-of-line annotations |
| `examples/trampoline`      | Overriding C++ virtuals from Python — generated pybind11 trampolines from `virtual_spec` |
| `examples/trampoline-node` | Overriding C++ virtuals from JavaScript — generated N-API trampolines from `virtual_spec` |
| `examples/moc`             | Qt-flavoured meta-object demo on `mini_moc.h` (properties + signals) |
| `examples/docgen`          | Reflection-driven Markdown / HTML reference generator |
| `examples/paraview`        | ParaView plugin property-panel XML from an annotated `vtkThreshold` spec (every backend feature) |
| `examples/qt`              | Building a Qt widget form from a reflected struct   |
| `examples/qml`             | Exposing a reflected C++ object to QML              |
| `examples/bindings/python` | Hand-written pybind11 backend (reference)           |
| `examples/bindings/node`   | Hand-written N-API backend (reference)              |
| `examples/bindings/julia`  | Hand-written CxxWrap/jlcxx backend (reference, requires CxxWrap.jl) |
| `examples/bindings/rest`   | Hand-written HTTP/REST backend (reference)          |
| `examples/bindings/web`    | Hand-written WebAssembly backend (requires reflection-aware emsdk) |

## Design notes

- [Quick start](docs/QUICKSTART.md) — five-step guide to generating bindings for an existing library
- [Extending](docs/EXTENDING_BACKEND.md) — how to extend the rosetta backend
<br><br>
- [Generate](docs/GENERATE.md) — full reference for `rosetta::generate`, the manifest schema, and the tool layering
- [Free functions](docs/FREE_FUNCTIONS.md) — sketch for reflecting namespace-scope functions
- [Other annotations](docs/OTHER_ANNOTATIONS.md) — proposed annotation kinds beyond the current three
- [Out-of-line annotations](docs/OUT_OF_LINE_ANNOTATIONS.md) — keep headers clean: a JSON side-car of annotations baked in at generation time, merged at compile time
- [Todo list](docs/TODO.md) — what the walker and visitor surface still miss (static data members, parameter metadata, nested types, ...)

## License

[MIT](./LICENSE)

## Author
[xaliphostes](https://github.com/xaliphostes)