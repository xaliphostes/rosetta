
# Rosetta — A C++ Automatic Language Binding

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
<p align="left" style="padding-left: 100px;">
  <img src="https://img.shields.io/static/v1?label=Python&logo=python&logoColor=white&message=support&color=success" alt="Python support"><br>
  <img src="https://img.shields.io/static/v1?label=Javascript&logo=javascript&logoColor=white&message=support&color=success" alt="JavaScript support"><br>
  <img src="https://img.shields.io/static/v1?label=Emscripten&logo=emscripten&logoColor=white&message=soon&color=informational" alt="Emscripten support"><br>
  <img src="https://img.shields.io/static/v1?label=Lua&logo=lua&logoColor=white&message=soon&color=informational" alt="Lua support"><br>
  <img src="https://img.shields.io/static/v1?label=Java&logo=java&logoColor=white&message=soon&color=informational" alt="Java support"><br>
  <img src="https://img.shields.io/static/v1?label=Ruby&logo=ruby&logoColor=white&message=soon&color=informational" alt="Ruby support"><br>
  <img src="https://img.shields.io/static/v1?label=Julia&logo=julia&logoColor=white&message=soon&color=informational" alt="Julia support">
</p>

---

## 🧩 Overview

**Rosetta** is a **non-intrusive C++ header-only introspection library** that automatically generates consistent bindings for Python, JavaScript, Lua, Ruby, Julia and more — without modifying your C++ code.
Write your classes once, and export them everywhere. You do not need to know anything about the underlaying libs that are used for the bindings (NAPI, Pybind11, Rice...)

Rosetta supports two complementary workflows:

1. **Direct C++ registration** using C++ introspection. This type of registration is fine grained and let you control everything
2. **Interface Description Language (IDL)** via YAML or JSON files (***STILL IN DEV MODE!***)

---

## ✨ Features

1. **Zero-intrusion** — No inheritance, no macros inside your classes, no wrapper
2. **Simple to use**
3. **One API -> Multi-language output** — Python (pybind11), JavaScript (N-API), Lua, WASM...
4. Supports:
   - **Multiple constructors**
   - **Inheritance & polymorphism** — Virtual methods, multiple inheritance
   - **Const correctness** — Differentiates const/non-const methods
   - **Functors**
   - **Virtual fields** - From `setDummy`/`getDummy` methods, create the virtual field `Dummy`
   - **Static methods**
   - **Free functions**
   - **Container** — `std::vector`, `std::map`, `std::set`, etc.
   - **Smart pointers** — `shared_ptr`, `unique_ptr`, raw pointers
5.  **Validation system** — Runtime constraints and checks
6.  **Serialization**
7.  **Documentation generation** — Markdown / HTML export
8.  **IDL language**

## Testing the lib

Go to the folder `examples/py/containers` (for example) and read [this file](./examples/README.md) before.

## [Short overview](SHORT_README.md)

## Contributing

Anyone is wellcome to write a generator, base on Rosetta introspection, for another scripting language (Lua, Julia, Ruby...).

See [this folder](include/rosetta/extensions/generators/) for **Python** and **JavaScript** generator.

## 📜 License

LGPL 3 License — see [LICENSE](LICENSE)

---

## 💡 Credits

[Xaliphostes](https://github.com/xaliphostes) (fmaerten@gmail.com)
