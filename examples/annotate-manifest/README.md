# annotate-manifest — out-of-line annotations via the manifest

Metadata (`doc`, `range`, `readonly`, `combobox`) lives in an **external JSON
file**, not in the header. The manifest's `annotations` field wires it in:
`rosetta_gen` bakes the side-car into the generated `bindings.h`, and every
backend sees the annotations. The class header stays completely clean.

## Files

| File | Role |
|---|---|
| `widget.h` | the class — **no annotations, never mentions rosetta** |
| `widget.ann.json` | the external annotation side-car, keyed by member name |
| `manifest.json` | wires them with one field: `"annotations": "widget.ann.json"` |
| `test.py` | asserts the file annotations reached the **Python** module (doc / readonly / range) |
| `test.js` | asserts the file annotations reached the **Node** module (readonly / range) |

`manifest.json` (the only wiring — note the `annotations` field):

```json
{
  "user_include": ".",
  "rosetta_include": "../../include",
  "generator_name": "widget_gen",
  "module_name": "widget",
  "targets": [ "markdown", "python", "node" ],
  "classes": [
    { "name": "Widget", "header": "widget.h", "annotations": "widget.ann.json" }
  ]
}
```

## Run it

```bash
# (one-time) build the framework scaffolder -> <repo>/bin/rosetta_gen
cmake -S ../../tools/rosetta_gen -B ../../tools/rosetta_gen/build
cmake --build ../../tools/rosetta_gen/build

# 1. generate the driver project (bindings.h has the baked annotations)
../../bin/rosetta_gen manifest.json generated

# 2. build it -> the `generator` binary is dropped in this folder
cmake -S generated -B generated/build && cmake --build generated/build

# 3. run it -> emits the backend projects (markdown/, python/, node/) under out/
./generator out

# 4. see the annotations, all sourced from widget.ann.json:
cat out/markdown/widget.md
```

Expected `out/markdown/widget.md`:

```markdown
# Widget

## Fields

| Name | Type | Description |
|------|------|-------------|
| `title` | `std::string` | The widget title |
| `count` | `int` | Visible items (range: 0..100) |
| `id` | `int` | _(readonly)_ |
| `mode` | `std::string` | (choices: fast, slow) |
```

`title`/`count` carry a `doc`, `count` a `range`, `id` is `readonly`, and `mode`
is a `combobox` — none of which appear in `widget.h`.

## Test the Python & Node bindings

The same side-car drives the compiled language modules too. `test.py` / `test.js`
build on the `out/python` and `out/node` projects emitted in step 3.

```bash
# Python (pybind11)
cmake -S out/python -B out/python/build && cmake --build out/python/build
python3 test.py
# -> test.py OK — doc, readonly and range from widget.ann.json all applied

# Node (N-API, built with cmake-js)
( cd out/node && npm install && npm run build )
node test.js
# -> test.js OK — readonly and range from widget.ann.json all applied
```

`test.py` checks that `Widget.title` has the docstring `"The widget title"`,
that `w.id = …` raises (read-only), and that `w.count = 999` raises while
`w.count = 50` works (range). `test.js` checks the read-only and range behaviour
(N-API has no property docstrings). All of it comes from `widget.ann.json`.

## Notes

- A class with no `annotations` field is simply un-annotated — the field is the
  single, optional entry point.
- The bytes are baked at generation time (no `#embed`, no extra compiler flags).
- Inline P3394 annotations, if present, are **merged** with the JSON ones rather
  than replaced.
- This works for **every** backend, not just markdown. Switch `targets` to e.g.
  `["python", "node", "openapi"]` and the same side-car becomes pybind/N-API
  docstrings + read-only properties + range checks, and OpenAPI
  `description`/`minimum`/`maximum`/`readOnly`/`enum`.

See [`docs/OUT_OF_LINE_ANNOTATIONS.md`](../../docs/OUT_OF_LINE_ANNOTATIONS.md).
