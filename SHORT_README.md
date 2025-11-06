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

## Introspection: register once with Rosetta (pure C++)
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
        register_classes();
        BIND_PY_CLASS(Vector3D);
    }
    END_PY_MODULE()
    ```

2. For JavaScript
    ```cpp
    BEGIN_JS_MODULE(rosetta_example) {
        register_classes();
        BIND_JS_CLASS(Vector3D);
    }
    END_JS_MODULE()
    ```

3. For...
   Just ask.

## Consequences

- Bind what you want (only the necessary C++ classes and functions)
- Bind for the language you want (Python, JavaScript, Lua, C#...)
- All bindings shared the same API
- C++ API changes? Recompile for all bindings in one go

The (non-intrusive) **introspection** is the key of this lib!
