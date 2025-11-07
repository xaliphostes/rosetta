# Rosetta very short

## Your C++ class

```cpp
class Vector3D {
public:
    double x, y, z;

    Vector3D();
    Vector3D(double x_, double y_, double z_);
    double length() const;
    void normalize();
};
```

## Introspection: register once with Rosetta (no other external libs)
```cpp
#include "rosetta/rosetta.h"

void register_classes() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}
```

## Then...bind everywhere!

1. For Python
    ```cpp
    BEGIN_PY_MODULE(rosetta_example) {
        register_classes(); // see above
        BIND_PY_CLASS(Vector3D);
    }
    END_PY_MODULE()
    ```

2. For JavaScript
    ```cpp
    BEGIN_JS_MODULE(rosetta_example) {
        register_classes(); // see above
        BIND_JS_CLASS(Vector3D);
    }
    END_JS_MODULE()
    ```

3. For...
   Just ask.

## Consequences

- Expose only what you need (C++ classes and functions)
- Bind to the language you want (Python, JavaScript, Lua, C#...)
- All bindings share the same API
- C++ API changes? Recompile all bindings in one go

The (non-intrusive) **introspection** is the key to this library!
