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

### 1. Make Your Class Introspectable

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

or if your class is already in a lib, make a wrapper around it.

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

### 3. Use Runtime Introspection (in C++)

```cpp
Person person("Alice", 30);

// Access members by name
person.printMemberValue("name");  // Prints: name (string): Alice
person.setMemberValue("age", 25);
std::cout << std::any_cast<int>(person.getMemberValue("age")); // Prints: 25

// Call methods by name
person.callMethod("introduce");   // Prints: Hi, I'm Alice
person.callMethod("setName", std::vector<std::any>{std::string("Bob")});
auto name = person.callMethod("getName");
std::cout << std::any_cast<std::string>(name); // Prints: Bob

// Introspection utilities
std::cout << person.getClassName();        // Prints: Person
std::cout << person.hasMember("name");     // Prints: 1 (true)
std::cout << person.hasMethod("introduce"); // Prints: 1 (true)
person.printClassInfo(); // Prints complete class information
```

## API Reference

### Introspectable Base Class

All introspectable classes inherit from `Introspectable` and gain these methods:

- `getMemberValue(name)` → `std::any` - Get member value by name
- `setMemberValue(name, value)` → `void` - Set member value by name
- `callMethod(name, args = {})` → `std::any` - Call method by name
- `hasMember(name)` → `bool` - Check if member exists
- `hasMethod(name)` → `bool` - Check if method exists
- `getMemberNames()` → `vector<string>` - Get all member names
- `getMethodNames()` → `vector<string>` - Get all method names
- `getClassName()` → `string` - Get class name
- `printMemberValue(name)` - Print member with type info
- `printClassInfo()` - Print complete class information

### TypeRegistrar Template

Fluent registration API:

- `member(name, &Class::member)` - Register a member variable
- `method(name, &Class::method)` - Register a method (supports 0-n parameters)

Both return `TypeRegistrar&` for method chaining.

## Compilation

Requires C++20 or later:

```cmake
cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)
add_executable(your_app main.cpp)
```

```bash
g++ -std=c++20 main.cpp -o your_app
```

## Supported Types

- All fundamental types (`int`, `double`, `float`, `bool`, etc.)
- `std::string`
- Custom classes (with appropriate `std::any` casting)
- Method parameters and return values (including `void`)

## Limitations

- Requires explicit registration of members/methods
- Runtime overhead due to `std::any` and function pointers
- No inheritance introspection (each class registers independently)

## License

LGPL License - feel free to use in your project