# Auto-docgen example

Emits Markdown documentation for the annotated `Person` struct from
`../bindings/person.h`, using `rosetta::generate_markdown<T>()` from
`<rosetta/docgen.h>`. Unlike the other examples, this one has **no**
external dependencies — just libc++.

## Build

```bash
cmake -G Ninja -B build
cmake --build build
./build/auto_docgen              # print to stdout
./build/auto_docgen > Person.md  # capture to a file
```

## Sample output

```markdown
# Person

## Fields

| Name | Type | Description |
|------|------|-------------|
| `name` | `std::string` | the person's display name |
| `age` | `int` | age in whole years (range: 0..150) |
| `id` | `std::string` | server-assigned identifier _(readonly)_ |

## Methods

### `greet(salutation: const std::string &) → std::string`

Returns a greeting prefixed by the given salutation.

### `clear() → void`
```

## Annotation surface

| Annotation     | Effect on the output                                  |
|----------------|-------------------------------------------------------|
| `doc{"..."}`   | description column for fields / paragraph for methods |
| `readonly`     | `_(readonly)_` tag in the description column          |
| `range{lo,hi}` | `(range: lo..hi)` tag in the description column       |

## What's where

| File                | Role                                                  |
|---------------------|-------------------------------------------------------|
| `main.cpp`          | Walks `Person`, writes the markdown to stdout         |
| `CMakeLists.txt`    | clang-p2996 wiring                                    |
| `../bindings/person.h` | Demo type — same one used by every other backend   |

The visitor itself lives in `include/rosetta/docgen.h` and is fewer
than 120 lines.
