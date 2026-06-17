# Trampoline example — overriding C++ virtuals from Python

This example shows what rosetta does when a reflected class has **virtual
methods**: the Python backend emits a [pybind11 *trampoline*][tramp] so a Python
subclass can override those virtuals and have C++ dispatch back into Python.

[tramp]: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python

## How it works

`shapes.h` is ordinary, unmodified C++:

```cpp
struct Shape {
    std::string label = "shape";
    virtual double      area() const = 0;                 // pure virtual
    virtual std::string describe() const { return label; } // virtual + default
    virtual ~Shape() = default;
};
```

`walk<Shape>()` reflects these methods. Because they are virtual it injects a
`rosetta::virtual_spec` into each method's annotation pack; the Python backend
reads that (plus the exact signature, `const`-ness, and pure-ness) and generates
`auto_pybind.cpp` containing a trampoline:

```cpp
namespace rosetta_py {
class Py_Shape : public Shape {
public:
    using Shape::Shape;
    double area() const override {
        PYBIND11_OVERRIDE_PURE(double, Shape, area, );   // pure -> _PURE
    }
    std::string describe() const override {
        PYBIND11_OVERRIDE(std::string, Shape, describe, ); // non-pure -> plain
    }
};
}

PYBIND11_MODULE(shapes, m) {
    rosetta::bind_pybind<Shape, rosetta_py::Py_Shape>(m, "Shape"); // class_<Shape, Py_Shape>
}
```

A class with **no** virtual methods gets no trampoline and binds as a plain
`py::class_<T>` — exactly as before.

## Build & run

```bash
cmake -G Ninja -B build && cmake --build build
python3 test_shapes.py    # -> trampoline OK: Python overrides dispatched through C++ virtuals
```

Requires the clang-p2996 fork at `$HOME/devs/c++/clang-p2996/build` and
`pip install pybind11`.

## What the test proves

- `Circle(Shape)` overrides the **pure virtual** `area()` and the virtual
  `describe()` — both reachable through the C++ base interface.
- `Square(Shape)` overrides only `area()`; `describe()` **falls back** to the
  C++ default body (`Shape::describe`).
- Calling the un-overridden pure virtual `Shape().area()` **raises** — the
  `PYBIND11_OVERRIDE_PURE` wiring.

`auto_pybind.cpp` is committed as generated (rosetta's Python backend produced
it); regenerate it with the `rosetta_gen` tool against a manifest listing
`Shape`, or via `rosetta::generate<Shape>(...)`.
