# Manifest-driven generation

This example shows the **manifest-driven** path of Rosetta: generate Python, Node, REST, and WebAssembly bindings for an existing class **without touching its source**. Instead of adding a `RTTR`-style registration block to the class, you point Rosetta at the header through a small `manifest.json` and let reflection do the rest.

It binds the same `Person` class used by [`examples/bindings`](../bindings) — but here nothing is written by hand: every file under [`generated/`](generated) is produced by the tooling.

## The two stages

Rosetta layers two tools, so generation happens in two steps:

```
manifest.json ──(rosetta_gen)──▶ generated/  ──(build + run)──▶ output/
                                  bindings.h                     python/
                                  reflected_person_gen.cpp       node/
                                  CMakeLists.txt                 rest/
                                                                 wasm/
```

1. **`rosetta_gen`** (`tools/rosetta_gen`) reads `manifest.json` and emits, for each declared class, a tiny *project-specific scaffolder*: the contents of `generated/`.
2. That scaffolder is a normal C++ program. Build it and run it, and `rosetta::generate<Person>` walks the reflected class and writes the actual per-backend binding projects under your chosen output directory.

## The manifest

[`manifest.json`](manifest.json) describes what to generate:

```json
{
  "user_include": "../bindings",
  "rosetta_include": "../../include",
  "generator_name": "reflected_person_gen",
  "targets": ["python", "node", "rest", "wasm"],
  "classes": [
    {
      "name": "Person",
      "header": "person.h"
    }
  ]
}
```

| Field             | Meaning                                                         |
| ----------------- | -------------------------------------------------------------- |
| `user_include`    | Include dir holding your class headers (resolved from the manifest) |
| `rosetta_include` | Path to the Rosetta `include/` directory                       |
| `generator_name`  | Base name for the generated scaffolder program / CMake target  |
| `targets`         | Backends to emit, shared by every class: `python`, `node`, `rest`, `wasm` |
| `classes[].header`| Header that declares the class (required)                      |
| `classes[].name`  | The C++ class to bind (optional — defaults to the header's basename) |

Each class's binding library / module name is derived as `reflected_<lowercase name>`.

## What `rosetta_gen` produces

Running `rosetta_gen` on the manifest writes the `generated/` directory (already committed here so you can read it without a compiler):

- **`bindings.h`** — a `rosetta::binding_info<Person>` specialization carrying the `targets`, `lib`, and `header` from the manifest.
- **`reflected_person_gen.cpp`** — a `main()` that parses `<dir>` and calls
  `rosetta::generate<Person>()`.
- **`CMakeLists.txt`** — builds that program with the clang-p2996 reflection flags (`-freflection -freflection-latest -fexperimental-library -fannotation-attributes`).

## Build & run the generated scaffolder

```bash
# 1. build the generated tool
cmake -G Ninja -S generated -B generated/build
cmake --build generated/build

# 2. run it → per-backend binding projects under output/
./generated/build/reflected_person_gen output
```

This drops one project per backend (`python/`, `node/`, `rest/`, `wasm/`) into `output/`, each ready to build the way the hand-written [`examples/bindings`](../bindings) backends are.

## Requirements

- A [clang-p2996](https://github.com/bloomberg/clang-p2996) build at `$HOME/devs/c++/clang-p2996/build` (or override `CLANG_P2996_ROOT`).
- CMake 3.28+ and Ninja (or Make).

## See also

- [`docs/GENERATE.md`](../../docs/GENERATE.md) — full reference for `rosetta::generate`, the manifest schema, and the tool layering.
- [`docs/QUICKSTART.md`](../../docs/QUICKSTART.md) — end-to-end walkthrough.
- [`examples/bindings`](../bindings) — the equivalent backends written by hand.
