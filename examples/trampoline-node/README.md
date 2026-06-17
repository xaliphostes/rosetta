# Trampoline example (Node) — overriding C++ virtuals from JavaScript

The N-API counterpart of [`examples/trampoline`](../trampoline) (pybind11).
When a reflected class has **virtual methods**, rosetta's Node backend emits a
trampoline so a JavaScript subclass can override them and have C++ dispatch back
into JS.

## How it works

`shapes.h` is ordinary, unmodified C++ (`Shape` with a pure virtual `area`, a
virtual `describe`, and a non-virtual `scaled_area` that calls `area()`).

N-API has no `PYBIND11_OVERRIDE` macro and `Wrap<T>` holds its value by
*composition*, so the mechanism differs from pybind:

- The generated `auto_napi.cpp` defines `class Js_Shape : public Shape, public
  rosetta::NapiTrampoline`. Each virtual override routes through
  `rosetta::napi_call_override[_pure]`.
- `Wrap<Shape, Js_Shape>` holds a `Js_Shape` (so its vtable can reach JS) and
  hands the trampoline a **weak** handle to the JS object.
- On a C++ virtual call, the trampoline asks: *did the JS object override this
  method?* It compares the function the JS object would invoke against the bound
  C++ prototype method (strict equality). Different ⇒ call the JS override;
  same ⇒ fall back to the C++ base (or throw, for a pure virtual). That identity
  check is the **recursion guard** — without it a non-override would bounce
  C++ → JS → bound C++ → trampoline → JS forever.

## Build & run

```bash
npm install            # node-addon-api + cmake-js
npx cmake-js compile
node test_shapes.js    # -> trampoline OK: JS overrides dispatched through C++ virtuals
```

Requires the clang-p2996 fork at `$HOME/devs/c++/clang-p2996/build`.

## What the test proves

- `Circle` overrides the pure virtual `area()` and the virtual `describe()`.
- `Square` overrides only `area()`; `describe()` **falls back** to the C++
  default (the recursion guard at work).
- `scaled_area()` — a non-virtual C++ method calling `area()` — reaches the JS
  override, proving **C++ → JS** dispatch.
- Calling the un-overridden pure virtual `new Shape().area()` **throws**.

## Limitations

The Node binding marshals user types **by value** (`from_napi` copies the
wrapped object out). A trampolined type passed *by value* to another C++
function is therefore sliced, losing the JS override — the trampoline only
preserves overrides for the wrapped object itself and for C++ code that calls
its virtuals through that object (as `scaled_area` does). This mirrors the
value-semantics design of the existing N-API backend.
