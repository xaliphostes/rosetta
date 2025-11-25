
# Rosetta — A C++ Introspection & Automatic Language Binding

# **One registration, infinite possibilities** 🚀

<p align="center">
  <img src="media/logo.png" alt="Logo rosetta" width="300">
</p>

<p align="center">
  <img src="https://img.shields.io/static/v1?label=Linux&logo=linux&logoColor=white&message=support&color=success" alt="Linux support">
  <img src="https://img.shields.io/static/v1?label=macOS&logo=apple&logoColor=white&message=support&color=success" alt="macOS support">
  <img src="https://img.shields.io/static/v1?label=Windows&logo=windows&logoColor=white&message=support&color=success" alt="Windows support"><br>
  <img src="https://img.shields.io/badge/C%2B%2B-20+-green.svg" alt="Language">
  <img src="https://img.shields.io/badge/license-LGPL-green.svg" alt="License">
</p>
With support to:
<p align="left">
<div style="padding-left: 50px;">
  <img src="https://img.shields.io/static/v1?label=Python&logo=python&logoColor=white&message=support&color=success" alt="Python support"><br>
  <img src="https://img.shields.io/static/v1?label=Javascript&logo=javascript&logoColor=white&message=support&color=success" alt="JavaScript support"><br>
  <img src="https://img.shields.io/static/v1?label=WebAssembly&logo=webassembly&logoColor=white&message=support&color=sucess" alt="Emscripten support"><br>
  <img src="https://img.shields.io/static/v1?label=Qt&logo=qt&logoColor=white&message=support&color=sucess" alt="Qt support"><br>
  <img src="https://img.shields.io/static/v1?label=Lua&logo=lua&logoColor=white&message=soon&color=informational" alt="Lua support"><br>
  <img src="https://img.shields.io/static/v1?label=Java&logo=java&logoColor=white&message=soon&color=informational" alt="Java support"><br>
  <img src="https://img.shields.io/static/v1?label=Ruby&logo=ruby&logoColor=white&message=soon&color=informational" alt="Ruby support"><br>
  <img src="https://img.shields.io/static/v1?label=Julia&logo=julia&logoColor=white&message=soon&color=informational" alt="Julia support">
  </div>
</p>

---

## 🧩 Overview

**Rosetta** is a **non-intrusive C++ header-only introspection library** that is used to automatically generates consistent bindings for Python, JavaScript, Lua, Ruby, Julia and more — without modifying your C++ code.

Describe your introspection once, and export them everywhere. You do not need to know anything about the underlaying libs that are used for the bindings (NAPI, Pybind11, Rice...)

Rosetta supports two complementary workflows:

1. **Direct C++ registration** using C++ introspection. This type of registration is fine grained and let you control everything
2. **Interface Description Language (IDL)** via YAML files (***STILL IN DEV MODE!***)

---

## ✨ Features

1. **Zero-intrusion** — No inheritance, no macros inside your classes, no wrapper
2. **Simple to use**
3. **One API -> Multi-language output** — Python (pybind11), JavaScript (N-API), Lua, WASM...
4. Supports:
   - **Multiple constructors**
   - **Inheritance & polymorphism** — Virtual methods, multiple inheritance
   - **Const correctness** — Differentiates const/non-const methods
   - **Method Complete Overload Access**
   - **Functors**
   - **Fields** - Member variables
   - **Virtual fields (`property`)** - From `setDummy`/`getDummy` methods, create the virtual field `dummy`
   - **Static methods**
   - **Free functions**
   - **STL Containers** — `vector`, `map`, `set`, `array`, etc... of any type
   - **Smart pointers** — `shared_ptr`, `unique_ptr`, raw pointers
5.  **Validation system** — Runtime constraints and checks
6.  **Serialization**
7.  **Documentation generation** — Markdown / HTML export
8.  **IDL language** - See [this example](./idl/example/geometry.yaml) 
9.  **Qt Integration** — QML bridge for dynamic property editing, automatic property editor widgets
10. **Undo/Redo extension** - Generic undo/redo manager for tracking and reverting property/field changes
  
## 📊 Comparison with Other Systems

| Feature | Rosetta | Qt MOC | RTTR | Boost.Describe |
|---------|---------|--------|------|----------------|
| Non-intrusive | ✅ | ❌ | ✅ | ✅ |
| No macros in class | ✅ | ❌ | ❌ | ❌ |
| Header-only | ✅ | ❌ | ❌ | ✅ |
| Static methods | ✅ | ✅ | ✅ | ⚠️ |
| Virtual methods | ✅ | ✅ | ✅ | ❌ |
| Free functions | ✅ | ❌ | ✅ | ❌ |
| Python bindings | ✅ | ✅ | ⚠️ | ❌ |
| Serialization | ✅ | ✅ | ✅ | ⚠️ |
| Qt integration | ✅ | ✅ | ⚠️ | ❌ |
| IDL support | 🚧 | ✅ | ❌ | ❌ |

### Legends

| Symbol | Meaning | Description |
|--------|---------|-------------|
| ✅ | **Fully Supported** | Feature is implemented, stable, and works as expected |
| ⚠️ | **Partial Support** | Feature exists but has limitations, requires workarounds, or is incomplete |
| ❌ | **Not Supported** | Feature is not available or not applicable |
| 🚧 | **In Progress** | Feature is being developed or planned |

## 🚀 Short overview

### 1. Your lib
```cpp
class Vector3D {...};
class SceneManager {...};
...
```
along with a static or dynamic library (if any).

### 2. Describe your API with Rosetta and generate the binding for any language 
This description provide the introspection of your classes that will be used by the generators (see below).
```cpp
#include <rosetta/rosetta.h>
#include <yourlib/all.h>

void rosetta_registration() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);

    ROSETTA_REGISTER_CLASS(SceneManager)
        .method("add", &SceneManager::add)
        ...;
}
```

### 3. Finally, generate the binding for any language
Since the introspection of your C++ classes (and free functions) is now created by Rosetta, binding is
straightforward as long as the generator for a given language is available:

1. For Python
    ```cpp
    #include <rosetta/extensions/generators/py_generator.h>

    BEGIN_PY_MODULE(rosetta_example) {
        rosetta_registration();
        BIND_PY_CLASSES(Vector3D, SceneManager);
    }
    END_PY_MODULE()
    ```
2. For JavaScript
    ```cpp
    #include <rosetta/extensions/generatorsv/js_generator.h>

    BEGIN_JS_MODULE(rosetta_example) {
        rosetta_registration();
        BIND_JS_CLASSES(Vector3D, SceneManager);
    }
    END_JS_MODULE()
    ```

3. For...??
   Just ask.

## 🖼️ Qt Integration

Rosetta provides seamless Qt6 integration with two complementary components:

### QML Bridge
The `QmlBridge` class exposes Rosetta-registered objects to QML, enabling dynamic property editing without compile-time type knowledge:
```qml
QmlBridge {
    id: bridge
}

// Bind any Rosetta-registered object
bridge.setObject("MyClass", myObject)

// Dynamically access fields and methods
var value = bridge.getField("position")
bridge.setField("scale", 2.0)
bridge.invokeMethod("reset", [])
```

### Automatic Property Editor

<p align="center">
  <img src="media/editor_properties.png" alt="Properties" width="300">
  <img src="media/editor_methods.png" alt="Methods" width="300">
</p>

Generate Qt widgets automatically from your Rosetta-registered classes:
```cpp
#include <rosetta/extensions/qt/qt_property_editor.h>
#include <rosetta/rosetta.h>

class Person {...};

static auto reg = []() {
    rosetta::core::Registry::instance()...
}();

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Person person;

    auto *inspector = new rosetta::qt::ObjectInspector<Person>(&person);
    inspector->setWindowTitle("Person Inspector");
    inspector->resize(300, 400);
    inspector->show();

    return app.exec();
}
```

The property editor supports:
- **Automatic widget generation** — Spinboxes for numbers, checkboxes for bools, line edits for strings
- **Custom widget registration** — Sliders, color pickers, file selectors, combo boxes
- **Grouped properties** — Organize fields into collapsible sections
- **Multi-object editing** — Edit multiple objects simultaneously
- **Method invocation** — Button panels for calling registered methods
- **Undo/redo support** — Track property changes for reversible editing

## 💡 Contribute Your Own Generator

You’re very welcome to create a generator based on **Rosetta introspection** for other scripting languages — such as **Lua**, **Julia**, or **Ruby**!

👉 Check out [this folder](include/rosetta/extensions/generators/) to see the existing **Python**, **JavaScript** and **emscripten** generators for inspiration.

Every new generator helps expand the ecosystem — contributions are always appreciated ❤️

## 📜 License

LGPL 3 License — see [LICENSE](LICENSE)

---

## 💡 Credits

[Xaliphostes](https://github.com/xaliphostes) (fmaerten@gmail.com)
