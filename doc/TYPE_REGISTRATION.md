# Integration Guide: Adding Type Registry to Your Project

## Option A: Using Macros (Recommended)

In your header files, add registration after class definition:

```cpp
// person.h
#include <rosetta/introspectable.h>

class Vector3D {
    float x, y, z;
    // ...
};

// Register immediately after definition
REGISTER_TYPE(Vector3D);

class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)
    // ...
private:
    Vector3D position;
};
```

## Option B: Central Registration File

Create a dedicated registration file:

```cpp
// types_init.h
#pragma once

void initialize_custom_types();

// types_init.cpp
#include "types_init.h"
#include <rosetta/type_registry.h>
#include "Vector3D.h"
#include "Color.h"
#include "Mesh.h"
// ... include all custom types

void initialize_custom_types() {
    REGISTER_TYPE(Vector3D);
    REGISTER_TYPE(Color);
    REGISTER_TYPE(Mesh);
    // ... register all types
}

// main.cpp
#include "types_init.h"

int main() {
    initialize_custom_types();  // Call at startup
    // ... rest of program
}
```

# Example Usage Patterns

## Pattern 1: Simple Project

```cpp
// main.cpp
#include <rosetta/introspectable.h>

// Define custom type
class Vector3D {
    float x, y, z;
};

// Register it
REGISTER_TYPE(Vector3D);

// Use it in introspectable class
class GameObject : public rosetta::Introspectable {
    INTROSPECTABLE(GameObject)
    Vector3D position;
};

void GameObject::registerIntrospection(rosetta::TypeRegistrar<GameObject> reg) {
    reg.member("position", &GameObject::position);  // Shows as "Vector3D"
}

int main() {
    GameObject obj;
    obj.printClassInfo();  // Clean type names!
    return 0;
}
```

## Pattern 2: Library with Public API

```cpp
// mylib/include/mylib/types.h
#pragma once
#include <rosetta/introspectable.h>

namespace mylib {

class Vector3D { /* ... */ };
class Color { /* ... */ };
class Mesh { /* ... */ };

// Public initialization function
void initialize_types();

} // namespace mylib

// mylib/src/types.cpp
#include "mylib/types.h"

namespace mylib {

REGISTER_TYPE(Vector3D);
REGISTER_TYPE(Color);
REGISTER_TYPE(Mesh);

void initialize_types() {
    // Types are registered via static initialization
    // This function can be empty or used for validation
}

} // namespace mylib

// client/main.cpp
#include <mylib/types.h>

int main() {
    mylib::initialize_types();  // Ensures types are registered
    // ... use library
}
```

## Pattern 3: Plugin System

```cpp
// plugin_interface.h
class Plugin {
public:
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

// my_plugin.cpp
#include <rosetta/type_registry.h>

class MyPlugin : public Plugin {
    void initialize() override {
        // Register plugin-specific types
        rosetta::TypeNameRegistry::instance().register_type<PluginVector>("PluginVector");
        rosetta::TypeNameRegistry::instance().register_type<PluginMesh>("PluginMesh");
    }
    
    void shutdown() override {
        // Cleanup if needed
    }
};
```

nregistered types fall back to `typeid(T).name()` (existing behavior)
- Registered types use clean names (new behavior)
- No breaking changes to existing APIs

# Testing Your Integration

## Test 1: Basic Registration

```cpp
#include <cassert>
#include <rosetta/type_registry.h>

class TestType { int x; };
REGISTER_TYPE(TestType);

void test_registration() {
    assert(rosetta::TypeNameRegistry::instance().is_registered<TestType>());
    assert(rosetta::TypeNameRegistry::instance().get_name<TestType>() == "TestType");
    std::cout << "✓ Basic registration works" << std::endl;
}
```

## Test 2: Type Name Resolution

```cpp
void test_type_names() {
    assert(rosetta::getTypeName<int>() == "int");
    assert(rosetta::getTypeName<std::string>() == "string");
    assert(rosetta::getTypeName<TestType>() == "TestType");
    std::cout << "✓ Type name resolution works" << std::endl;
}
```

## Test 3: Introspection Integration

```cpp
class TestObject : public rosetta::Introspectable {
    INTROSPECTABLE(TestObject)
    TestType member;
};

void TestObject::registerIntrospection(rosetta::TypeRegistrar<TestObject> reg) {
    reg.member("member", &TestObject::member);
}

void test_introspection() {
    TestObject obj;
    const auto* member = obj.getTypeInfo().getMember("member");
    assert(member->type_name == "TestType");
    std::cout << "✓ Introspection integration works" << std::endl;
}
```

## Test 4: JSON Output

```cpp
void test_json_output() {
    TestObject obj;
    std::string json = obj.toJSON();
    assert(json.find("\"type\": \"TestType\"") != std::string::npos);
    std::cout << "✓ JSON output has clean names" << std::endl;
}
```

## Run All Tests

```cpp
int main() {
    test_registration();
    test_type_names();
    test_introspection();
    test_json_output();
    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}
```

# Common Issues and Solutions

## Issue 1: Type name still mangled

**Cause**: Type not registered or registered after use

**Solution**:
```cpp
// ❌ Wrong order
class MyClass {
    Vector3D pos;
};
REGISTER_TYPE(Vector3D);  // Too late!

// ✅ Correct order
REGISTER_TYPE(Vector3D);
class MyClass {
    Vector3D pos;  // Now clean!
};
```

## Issue 2: Template type not working

**Cause**: Template types need explicit instantiation registration

**Solution**:
```cpp
template<typename T> class Container { };

// Must register each instantiation
REGISTER_TYPE_AS(Container<int>, "Container<int>");
REGISTER_TYPE_AS(Container<float>, "Container<float>");
```

## Issue 3: Linking errors with macros

**Cause**: Multiple definition of static registration object

**Solution**: Put registration in a single .cpp file, not in headers (unless header-only)

```cpp
// vector3d.h
class Vector3D { };

// vector3d.cpp - Register here, not in header
#include "vector3d.h"
REGISTER_TYPE(Vector3D);
```

# Performance Impact

- **Compile time**: Negligible (one hash table lookup)
- **Runtime**: O(1) lookup via `std::type_index`
- **Memory**: ~40 bytes per registered type (string + hash entry)
- **Startup**: One-time registration cost (microseconds per type)

Happy coding! 🚀