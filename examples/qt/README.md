# Auto-Qt-Widgets example

A QtWidgets property / method inspector built by **one** `rosetta::walk`
over the annotated `Person` struct from `../bindings/person.h`. Unlike
the `../qml/` example, this version has no QML and no metadata
intermediate — the visitor constructs `QSpinBox`, `QLineEdit`,
`QCheckBox`, and `QPushButton` rows directly and wires lambdas into
`QPushButton::clicked`.

```
┌──────────────────────────────────────────────────────┐
│ Person — generated from one C++26 reflection walk    │
├─[ Fields ]───────────────────────────────────────────┤
│  name           [Alice            ]   GET   PUT      │
│  age [0..150]   [ 42 ▲▼           ]   GET   PUT      │
│  id             [s-123            ]   GET   ro       │
├─[ Methods ]──────────────────────────────────────────┤
│  greet(1)       [salutation       ]         Call     │
│  clear(0)                                   Call     │
├──────────────────────────────────────────────────────┤
│  log...                                              │
└──────────────────────────────────────────────────────┘
```

Annotation behaviour mirrors the python / rest / qml backends:

| Annotation     | Effect on the UI                                       |
|----------------|--------------------------------------------------------|
| `doc{...}`     | tooltip on the field / method label                    |
| `readonly`     | editor disabled, **PUT** button replaced by **ro**     |
| `range{lo,hi}` | spinbox bounds + range hint on the label + PUT re-check|

## Build

```bash
cmake -G Ninja -B build
cmake --build build
./build/auto_qt
```

The CMake script defaults to:

- `CLANG_P2996_ROOT = $HOME/devs/c++/clang-p2996/build`
- `QT_DIR          = $HOME/Qt/6.5.3/macos`

Override either with `-D<name>=...` on the cmake line.

## What's where

| File                  | Role                                                          |
|-----------------------|---------------------------------------------------------------|
| `widget_visitor.h`    | `rosetta::WidgetVisitor<T>` + `build_inspector<T>` entry point|
| `main.cpp`            | Walks `Person`, shows the inspector                           |
| `CMakeLists.txt`      | Qt6::Widgets + clang-p2996 wiring                             |

## Notes on the build

- We define no `Q_OBJECT` class of our own — Qt's widget signals
  (`QPushButton::clicked`, etc.) are enough. AUTOMOC is still on because
  Qt's own widget headers contain `Q_OBJECT`.
- Reflection flags are scoped to `main.cpp` via
  `set_source_files_properties`, so moc-generated TUs stay on plain
  C++26.
