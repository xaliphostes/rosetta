# mini_moc example — Thermostat

A small Qt-flavoured demo built on `<rosetta/mini_moc.h>`. A `Thermostat`
model exposes annotated properties and signals; a `Display` prints state
changes and a `Heater` reacts to temperature/target updates. No moc, no
`.moc` file, no codegen step — everything is driven at compile time by
C++26 reflection.

## Build

```bash
cmake -G Ninja -B build
cmake --build build
./build/thermostat
```

## What it shows

- `[[= property{"handle", "notifySignal"}]]` on data members, accessed via
  `rosetta::moc::get<"handle">(obj)` / `set<"handle">(obj, v)`.
- `Signal<...>` members (signals by type — no annotation) + `[[= slot]]` methods,
  wired with `rosetta::moc::connect<"sig", "slot">(sender, receiver)`.
- **Multicast**: `temperatureChanged` reaches both the `Display` and the
  `Heater` from a single emit.
- **Equality gating**: writing the same value twice does not re-fire the
  signal.
- A property with **no NOTIFY** (`serial`) still works for read/write — it
  just doesn't multicast.
- **Compile-time checking**: the bottom of `main.cpp` has three commented-out
  lines you can uncomment one at a time to see a typo'd signal name, a
  type-mismatched connect, and a typo'd property name each fail at compile.

## Sample output

```
initial state: room=20.0°C target=21.0°C mode=off serial=TH-0001

-- switch on --
  [display] mode = heat

-- nudge target up to 22.5 --
  [display] target set to 22.5°C
  [heater] new target 22.5°C cached

-- sensor reads 19.8 (heater should kick on) --
  [display] room is now 19.8°C
  [heater] ON (cycle #1)

-- sensor reads 19.8 again (equality-gated, silent) --

-- sensor reads 22.6 (heater off) --
  [display] room is now 22.6°C
  [heater] OFF (cycle #2)

-- update serial (no NOTIFY declared, no signals fire) --

final: heater on=false cycles=2 serial=TH-0042
```

## What's where

| File             | Role                                                     |
|------------------|----------------------------------------------------------|
| `main.cpp`       | `Thermostat` + `Display` + `Heater` wired via `mini_moc` |
| `CMakeLists.txt` | clang-p2996 wiring                                       |
| `../../include/rosetta/mini_moc.h` | The reflection-driven moc itself       |

See also `tests/moc.cpp` for the minimal feature-by-feature exercise.
