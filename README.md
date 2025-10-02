# Rosetta, a C++ automatic language binding

<p align="center">
  <img src="media/logo.png" alt="Logo rosetta" width="300">
</p>

<p align="center">
  <img src="https://img.shields.io/static/v1?label=Linux&logo=linux&logoColor=white&message=support&color=success" alt="Linux support">
  <img src="https://img.shields.io/static/v1?label=macOS&logo=apple&logoColor=white&message=support&color=success" alt="macOS support">
  <img src="https://img.shields.io/static/v1?label=Windows&logo=windows&logoColor=white&message=support&color=sucess" alt="Windows support">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20+-blue.svg" alt="Language">
  <img src="https://img.shields.io/badge/license-LGPL-blue.svg" alt="License">
</p>

A lightweight, header-only C++ introspection system that enables runtime inspection and manipulation of class members and methods without external dependencies.

This C++ introspection system enables **automatic language binding generation for multiple scripting languages** without requiring manual binding code for each class member and method.

- For **Python**, the introspection data can drive automatic pybind11 binding generation, creating properties for all members and properly typed method bindings with argument validation - eliminating the need to manually write .def() calls for each class feature. Example:
  ```cpp
  PYBIND11_MODULE(introspection_demo, m) {
      rosetta::PyGenerator generator(m);
      generator.bind_classes<Person, Vehicle>();
  }
  ```

- For **JavaScript** (via V8, Node.js addons, or Emscripten), the reflection data enables automatic generation of property descriptors and method wrappers, allowing seamless integration where JavaScript objects can directly access C++ class members as properties and call methods with automatic type conversion between JavaScript values and C++ types. Example:
  ```cpp
  Napi::Object Init(Napi::Env env, Napi::Object exports) {
      rosetta::JsGenerator generator(env, exports);
      generator.bind_classes<Person, Vehicle>();
      return exports;
  }
  ```

The key advantage is that once a C++ class inherits from Introspectable and registers its members/methods, it can be automatically exposed to all scripting languages using the same introspection metadata, drastically reducing the maintenance burden of keeping multiple language bindings synchronized with C++ class changes.

## Features

- **Runtime Member Access**: Get/set member variables by name
- **Runtime Method Invocation**: Call methods by name with parameters
- **Type-Safe**: Compile-time registration with runtime type checking
- **Template-Based**: Clean, fluent registration API using member/method pointers
- **Zero Dependencies**: No external libraries required
- **C++20 Compatible**: Uses modern C++ features like `std::any` and `if constexpr`
- **Multiple constructors** if **needed**

## Quick Start

### 1. Make Your Class Introspectable (or wrap it)

```cpp
#include <rosetta/rosetta.h>

class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)    
public:
    Person(const std::string& n, int a) : name(n), age(a) {}
    
    std::string getName() const { return name; }
    void setName(const std::string& n) { name = n; }
    int getAge() const { return age; }
    void introduce() { std::cout << "Hi, I'm " << name << std::endl; }

private:
    std::string name;
    int age;
};
```

### 2. Register Ctors, Members and Methods

```cpp
void Person::registerIntrospection(rosetta::TypeRegistrar<Person> reg) {
    reg.constructor<>()
       .constructor<const std::string&, int, double>()
       .member("name", &Person::name)
       .member("age", &Person::age)
       .method("getName", &Person::getName)
       .method("setName", &Person::setName)
       .method("getAge", &Person::getAge)
       .method("introduce", &Person::introduce);
}
```

### 3. Bind in Js

```cpp
#include <rosetta/JsGenerator.h>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);
    generator.bind_class<Person>();
    return exports;
}

NODE_API_MODULE(jsperson, Init)
```

### 4. Bind in Py

```cpp
#include <rosetta/PyGenerator.h>

PYBIND11_MODULE(rosettapy, m) {
    rosetta::PyGenerator generator(m);
    generator.bind_class<Person>();
}
```

## Limitations

- Requires explicit registration of members/methods
- Runtime overhead due to `std::any` and function pointers
- No inheritance introspection (each class registers independently)

## License

LGPL License - feel free to use in your project