# Auto-QML example

A Qt Quick inspector window driven by **one** `rosetta::walk` over the
annotated `Person` struct from `../bindings/person.h`. The C++ side
emits metadata for each field and method into a generic
`ReflectedObject`; the QML side renders the UI from that metadata —
there is no hand-written, per-`Person` markup in `Main.qml`.

```
┌──────────────────────────────────────────────────────┐
│ Person — generated from one C++26 reflection walk    │
├──────────────────────────────────────────────────────┤
│ FIELDS                                               │
│   name              [Alice           ]  GET   PUT    │
│   age [0..150]      [42 ▲▼           ]  GET   PUT    │
│   id                [s-123           ]  GET   ro     │
├──────────────────────────────────────────────────────┤
│ METHODS                                              │
│   greet(1)          [Hello   ]            Call       │
│   clear(0)                                Call       │
├──────────────────────────────────────────────────────┤
│  log...                                              │
└──────────────────────────────────────────────────────┘
```

Annotation behaviour mirrors the python / rest backends:

| Annotation     | Effect on the UI                                  |
|----------------|---------------------------------------------------|
| `doc{...}`     | tooltip on the field / method label               |
| `readonly`     | editor disabled, **PUT** replaced with **ro**     |
| `range{lo,hi}` | shown next to label; invalid values logged in red |

## Build

```bash
cmake -G Ninja -B build
cmake --build build
./build/auto_qml
```

The CMake script defaults to:

- `CLANG_P2996_ROOT = $HOME/devs/c++/clang-p2996/build`
- `QT_DIR          = $HOME/Qt/6.5.3/macos`

Override either with `-D<name>=...` on the cmake line.

## What's where

| File                  | Role                                                          |
|-----------------------|---------------------------------------------------------------|
| `qml_visitor.h`       | `rosetta::QmlVisitor<T>` + `bind_qml<T>` entry point          |
| `reflected_object.h`  | The `Q_OBJECT` adapter exposed to QML (no template parameter) |
| `main.cpp`            | Walks `Person`, registers the wrapper, launches QML           |
| `Main.qml`            | Generic inspector — one `Repeater` over fields / methods      |
| `CMakeLists.txt`      | Qt6 + clang-p2996 wiring                                      |

## Notes on the build

- Reflection flags (`-freflection -freflection-latest -fannotation-attributes`)
  are scoped to `main.cpp` via `set_source_files_properties`, so moc-generated
  code is compiled with plain C++26.
- We link against the libc++ that ships with clang-p2996 (`-nostdlib++ -lc++ -lc++abi`).
  If Qt was built against a different libstdc++/libc++, you may see ABI warnings on
  symbols that cross the boundary — none currently do here because every type passed
  to Qt is a Qt type (`QString`, `QVariant`, …).
