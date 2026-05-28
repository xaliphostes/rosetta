---
marp: true
theme: default
paginate: true
header: "Rosetta · C++26 reflection in anger"
footer: "fmaerten@gmail.com"
class: invert
style: |
  section { font-size: 24px; }
  section.lead h1 { font-size: 64px; }
  section.lead p { font-size: 28px; }
  code { font-size: 0.85em; }
  pre { font-size: 0.7em; line-height: 1.25; }
  table { font-size: 0.8em; }
---

<!-- _class: lead -->

![w:260](../media/logo-rosetta.png)

# Rosetta

### One C++ class, many worlds — driven by C++26 reflection.

---

## The pitch

A C++ class is a contract: name, fields, methods, types, docs, units.

Today we **retype that contract** for every consumer — Python wrappers, Qt
widgets, REST handlers, doc pages, QML models, validators…

C++26 reflection lets the compiler hand us the contract directly.

> One `consteval` walk → all the backends.

---

## What lands in C++26

| Paper      | What it gives us                                        |
|------------|---------------------------------------------------------|
| **P2996**  | `^^T` reflection operator, `[: r :]` splice, `std::meta::*` |
| **P3394**  | `[[= rosetta::doc{"..."}]]` annotation attributes       |
| **P3294**  | Token injection — emit code from a `consteval` block    |

Rosetta uses P2996 + P3394 today. P3294 is on deck (see `mini_moc.h` notes).

---

## Compiler reality check

- Bloomberg **clang-p2996 fork** — what Rosetta is built and tested on.
- EDG — has the most complete set, including injection.
- GCC / mainline Clang / MSVC — not yet.

```bash
$HOME/devs/c++/clang-p2996/build/bin/clang++ \
  -std=c++26 \
  -freflection -freflection-latest \
  -fannotation-attributes \
  -fexperimental-library
```

Not shippable. Very fun.

---

## What's in the box

```
include/rosetta/
  annotations.h   doc / range / readonly — shared structural types
  walk.h          consteval walk<T>(visitor) over fields + methods
  docgen.h        a visitor that emits Markdown reference docs
  mini_moc.h      signals / slots / properties on top of reflection

examples/         qt, qml, docgen, moc, bindings/{python,node,rest,web}
tests/            one .cpp per concept, each builds to a runnable demo
```

The whole core is **~400 lines of headers**.

---

## The annotation surface

```cpp
struct doc {
    const char* text;
    consteval doc(const char* t)
      : text(std::define_static_string(t)) {}
    bool operator==(const doc&) const = default;
};

struct range { double min, max; /* + ==  */ };
struct readonly { /* + == */ };
```

Constraints: must be **structural** + reach static storage with linkage
(hence `define_static_string`). Anything else can't be an annotation.

---

## Annotated class

```cpp
struct Person {
    [[= rosetta::doc{"display name"}]]
    std::string name;

    [[= rosetta::doc{"age in years"}, = rosetta::range{0, 150}]]
    int age = 0;

    [[= rosetta::doc{"server id"}, = rosetta::readonly{}]]
    std::string id;

    [[= rosetta::doc{"Greet with the given salutation."}]]
    std::string greet(std::string const& s) const;
};
```

No registration, no macros. The annotations *are* the metadata.

---

## The walk

A single entry point. Visitors receive each member with its full
annotation pack as `auto...` non-type template parameters.

```cpp
template <typename T, typename Visitor> void walk(Visitor& v) {
    template for (constexpr auto fld : nonstatic_data_members_of(^^T, ctx)) {
        constexpr auto anns = annotations_of(fld);
        v.template field<fld, [:anns[Is]:]...>(name);
    }
    template for (constexpr auto fn : members_of(^^T, ctx)) {
        if constexpr (is_exportable_member_function(fn)) {
            v.template method_instance<fn, [:anns[Is]:]...>(name);
        }
    }
}
```

Every backend is **just a visitor**.

---

## Backend tour

| Visitor                 | What it produces                          |
|-------------------------|-------------------------------------------|
| `docgen.h`              | Markdown reference pages                  |
| `examples/qt`           | A Qt `QFormLayout` of editors             |
| `examples/qml`          | A `QObject` exposed to QML, properties & all |
| `bindings/python`       | pybind11 module                           |
| `bindings/node`         | N-API native module                       |
| `bindings/rest`         | HTTP routes (`GET /person/{id}`...)       |
| `bindings/web`          | Emscripten / WebAssembly module           |
| `mini_moc.h`            | Qt-style signals / slots / properties     |

---

## Spotlight: docgen

```cpp
#include <rosetta/docgen.h>
std::cout << rosetta::generate_markdown<Person>();
```

Output:

```markdown
# Person
## Fields
| Name | Type        | Description                              |
|------|-------------|------------------------------------------|
| name | std::string | display name                             |
| age  | int         | age in years (range: 0..150)             |
| id   | std::string | server id _(readonly)_                   |

## Methods
### `greet(s: const std::string&) → std::string`
Greet with the given salutation.
```

---

## Spotlight: `mini_moc`

Qt's `moc` is a separate code generator. It parses headers, emits `.moc`
files, links them in. It's been a Qt fixture for 25 years.

C++26 reflection makes it… optional.

```cpp
class Thermostat {
public:
    [[= moc::signal]] moc::Signal<double> temperatureChanged;

    [[= moc::property{"temperature", "temperatureChanged"}]]
    double m_temperature = 20.0;
};
```

No `.moc` file. No build step. Just a header.

---

## `mini_moc`: type-safe connect

```cpp
Thermostat th;
Display    d;

moc::connect<"temperatureChanged", "showTemperature">(th, d);
```

Three failure modes, **all at compile time**:

- typo'd signal name → `static_assert`: "no `[[=signal]]`-tagged member"
- typo'd slot name  → `static_assert`: "no `[[=slot]]`-tagged member"
- mismatched types  → template error at the lambda body

Qt's `SIGNAL()/SLOT()` macro form is string-checked at runtime. The
pointer-to-member form helps but still doesn't catch name typos.

---

## `mini_moc`: properties with NOTIFY

```cpp
moc::set<"temperature">(th, 19.8);  // writes m_temperature,
                                    // fires temperatureChanged(19.8)
moc::set<"temperature">(th, 19.8);  // equality-gated, silent
```

The setter is **automatic**:

- finds the field via the property's `name` annotation
- compares old vs new (Qt convention people forget half the time)
- splices the NOTIFY signal name to fire it
- all resolved at compile time

No injection (P3294 isn't in our compiler yet), so accessors live as
`get<>/set<>` free functions instead of `name()/setName()` methods.

---

## What you get vs. Qt moc

|                              | Qt moc            | rosetta `mini_moc`       |
|------------------------------|-------------------|--------------------------|
| External tool                | yes (`moc`)       | no                       |
| Name typos                   | runtime           | compile time             |
| Arg-type mismatch            | runtime (mostly)  | compile time             |
| Property change-gating       | by hand           | automatic                |
| QML / scripting integration  | yes               | not yet (build on top)   |
| Cross-thread queued dispatch | yes               | not yet (orthogonal)     |

Trade is honest: simpler core, narrower scope.

---

## What the walk still misses

(from `docs/TODO.md`)

- **Enums** — no `walk_enum<E>`; can't emit IntEnum / TS unions / REST enums.
- **Bases** — `nonstatic_data_members_of` is non-recursive.
- **Constructors** — filtered out today; every binding needs them.
- **Static data members** — not surfaced.
- **Nested types**, **free functions**, **operators** — invisible.
- **Method qualifiers** — `const`, `&&`, `noexcept`, `virtual` lost.
- **Parameter metadata** — names, defaults, per-parameter annotations.

The visitor concept also assumes one shape per annotation kind. Replacing
that with an annotation-pack model is the live refactor.

---

## Why this matters

For decades, C++ has had **two grades** of metadata:

1. What the compiler knows.
2. What you re-type for every binding generator, doc tool, ORM, serializer,
   GUI builder, RPC framework, fuzzer harness…

C++26 collapses (2) into (1).

Rosetta is a small proof: with a few hundred lines of headers and a clang
fork, you can already point a single class at seven different ecosystems.

---

<!-- _class: lead -->

## Try it

```bash
git clone <repo>
cd rosetta2/tests
cmake -B build && cmake --build build
./build/moc
./build/docgen
```

```bash
cd ../examples/moc
cmake -G Ninja -B build && cmake --build build
./build/thermostat
```

Source: `include/rosetta/` · Demos: `examples/` · Tests: `tests/`

P2996 / P3394 / P3294 — `wg21.link`
