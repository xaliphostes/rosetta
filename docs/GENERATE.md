# `rosetta::generate` — reflection-driven binding scaffolder

End-to-end workflow:

1. The user edits **`manifest.json`** (the only hand-written file).
2. The framework **`rosetta_gen`** tool reads it and emits
   `bindings.h`, `<lib>_gen.cpp`, and `CMakeLists.txt`. `<lib>` is the
   first class's `lib` field — so e.g. `lib: "reflected_person"`
   produces `reflected_person_gen.cpp` and a `reflected_person_gen`
   binary.
3. The generated **`<lib>_gen`** is built — uses C++26 reflection on
   the listed classes.
4. Running `<lib>_gen --out <dir>` emits a complete per-backend project
   tree (source + CMake + package.json + README).
5. Each generated backend can then be built independently.

The user's class headers are never modified.

## Layout

```
examples/
  bindings/
    person.h                             # user-owned, pristine
  generate/
    manifest.json                        # 1. user-edited input
    generated/                           # 2. produced by rosetta_gen ↓
      bindings.h                         #    rosetta::binding_info<T> specializations
      <lib>_gen.cpp                  #    main() with one generate<T> per class
      CMakeLists.txt                     #    builds <lib>_gen
    output/                              # 4. produced by <lib>_gen ↓
      python/  node/  rest/  web/        #    per-backend project trees
tools/
  rosetta_gen/                           #    the framework JSON → C++ tool
    rosetta_gen.cpp
    CMakeLists.txt
```

Two tools, two layers:

| Tool | Compiled with | Reads | Writes |
|---|---|---|---|
| `rosetta_gen` (framework, ships in repo) | system C++17 | `manifest.json` | `bindings.h`, `<lib>_gen.cpp`, `CMakeLists.txt` |
| `<lib>_gen` (generated, per project) | clang-p2996 (C++26 reflection) | the user's classes via reflection | per-backend project trees |

## 1. `manifest.json`

```json
{
  "user_include": "../bindings",
  "rosetta_include": "../../include",
  "classes": [
    {
      "name": "Person",
      "header": "person.h",
      "lib": "reflected_person",
      "targets": ["python", "node", "rest", "web"]
    }
  ]
}
```

Top-level fields:

| Field | Meaning |
|---|---|
| `user_include` | path to the directory containing the class headers — relative to `manifest.json`, or absolute. Resolved to an absolute path by `rosetta_gen`. |
| `rosetta_include` | path to rosetta's `include/` directory — same rules. |
| `classes` | array of per-class entries (below). |

The CMake target / binary name comes from the **first class's `lib`** —
`lib: "reflected_person"` produces a `reflected_person_gen` binary.

Per-class entry:

| Field | Meaning |
|---|---|
| `name` | C++ type name (must be reachable from `bindings.h` after `#include "<header>"`) |
| `header` | filename written into `#include "..."` |
| `lib` | library / module name baked into the generated bindings |
| `targets` | array of `"python"`, `"node"`, `"rest"`, `"web"` |

## 2. Build & run `rosetta_gen` (the framework tool)

`rosetta_gen` is system-C++ only (no reflection, no clang-p2996
needed). It pulls `nlohmann/json` via FetchContent.

```bash
cmake -G Ninja -S tools/rosetta_gen -B tools/rosetta_gen/build
cmake --build tools/rosetta_gen/build
./tools/rosetta_gen/build/rosetta_gen examples/generate/manifest.json
```

Output (under `examples/generate/generated/`) for `lib: "reflected_person"`:

- `bindings.h` — `rosetta::binding_info<T>` specialization per class.
- `reflected_person_gen.cpp` — `main()` that calls
  `rosetta::generate<T>(opt)` for every class, with `user_include` /
  `rosetta_include` already baked in as absolute paths.
- `CMakeLists.txt` — builds `reflected_person_gen` with the clang-p2996
  reflection flags and the two include directories.

## 3. Build the generated `<lib>_gen`

```bash
cmake -G Ninja -S examples/generate/generated -B examples/generate/generated/build
cmake --build examples/generate/generated/build
```

## 4. Run `<lib>_gen` — emit the per-backend projects

```bash
./examples/generate/generated/build/reflected_person_gen --out examples/generate/output
```

Result:

```
examples/generate/output/
  python/  auto_pybind.cpp     CMakeLists.txt              README.md
  node/    auto_napi.cpp       CMakeLists.txt  package.json README.md
  rest/    auto_rest.cpp       CMakeLists.txt              README.md
  web/     auto_emscripten.cpp CMakeLists.txt              README.md
```

## 5. Build any one backend

```bash
cd examples/generate/output/python
cmake -G Ninja -B build && cmake --build build
python test_reflected_person.py    # ship your own test
```

Other backends follow their usual conventions (`npm install` +
`cmake-js` for node, `emcmake cmake …` for web, FetchContent for rest).

## Adding a class

1. Add an entry to `classes[]` in `manifest.json`.
2. Re-run `rosetta_gen examples/generate/manifest.json`.
3. Rebuild `<lib>_gen`, then re-run it.

That's it — no C++ to hand-edit.

## How the layers fit

- **`rosetta_gen` (framework)** is plain text templating: read JSON,
  emit strings. No reflection. Uses `nlohmann/json` and `<sstream>`.
- **`<lib>_gen` (generated)** calls `rosetta::generate<T>(opt)` per
  class. `generate<T>` reads the `binding_info<T>` trait (lib name,
  header, targets) and walks `T` via `rosetta::walk` to produce the
  README body via `docgen::generate_markdown<T>`. The per-backend
  templates emit `auto_<backend>.cpp` + CMake + package.json.
- **The generated backends** are ordinary standalone CMake projects
  that compile the user's class against rosetta's binding kit.

## Known gaps

- Paths in `manifest.json` resolve relative to that file's location.
  Move the manifest and you must re-run `rosetta_gen`.
- `node` / `web` / `rest` are scaffolded but were not built
  end-to-end during the prototype — only `python` was verified.
  The templates mirror the working hand-written examples.
- A class missing from `manifest.json` is invisible; a class listed
  but with no member of `targets` matched (`"python"`, `"node"`,
  `"rest"`, `"web"`) silently emits nothing for that class.
