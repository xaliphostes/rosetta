# Automatic JavaScript Bindings for C++ Introspection

This directory contains an automatic binding generator that creates Node.js native addons from C++ classes using the introspection system. Just like the Python version, it requires minimal code to expose entire C++ classes to JavaScript.

## Features

- ✅ Natural JavaScript property access: person.name = "Alice"
- ✅ Method calls: person.introduce()
- ✅ Full introspection from JavaScript
- ✅ Automatic type conversion
- ✅ Automatic Property Binding: All class members become JavaScript properties with natural get/set syntax
- ✅ Multiple constructors binding
- ✅ Method Binding: All class methods become JavaScript functions with automatic parameter conversion
- ✅ Type Safety: Automatic type conversion between JavaScript and C++ types
- ✅ Introspection Utilities: Access to reflection information from JavaScript
- ✅ Error Handling: Proper JavaScript exceptions for invalid operations
- ✅ Factory Functions: Automatic creation of object factory methods

## Building

### Prerequisites

- Node.js (14 or later)
- npm
- C++20 compatible compiler
- Python (for node-gyp)

### Build Steps

```bash
# Install dependencies
npm install

# Build the addon
npm run build

# Run tests
npm test
```

## Quick Start

### 1. Define Your C++ Classes

```cpp
class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)    
public:
    Person();
    Person(const std::string &n, int a, double h); 
    std::string getName() const;
    void setName(const std::string& n);
    void introduce();

private:
    std::string name;
    int age;
    double height;
};

void Person::registerIntrospection(rosetta::TypeRegistrar<Person> reg) {
    reg.constructor<>()
       .constructor<const std::string&, int, double>()
       .member("name", &Person::name)
       .member("age", &Person::age)
       .member("height", &Person::height)
       .method("getName", &Person::getName)
       .method("setName", &Person::setName)
       .method("introduce", &Person::introduce);
}
```

### 2. Create Node.js Addon (Only 1 line!)

```cpp
#include <rosetta/generators/js.h>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    JavascriptBindingGenerator(env, exports).bind_class<Person>();
    return exports;
}

NODE_API_MODULE(introspection_demo, Init)
```

### 3. Use from JavaScript

```javascript
const introspection = require('./build/Release/introspection_demo');

// Create objects
const person = new introspection.Person("Alex", 22, 174);

// Natural property access
person.name = "Alice";
person.age = 30;
console.log(person.name);  // "Alice"

// Method calls
person.introduce();
const description = person.getDescription();

// Introspection
console.log(person.getClassName());     // "Person"
console.log(person.getMemberNames());   // ["name", "age", "height"]
console.log(person.getMethodNames());   // ["introduce", "getName", ...]

// Dynamic access
person.setMemberValue("age", 25);
console.log(person.getMemberValue("age"));  // 25

// JSON export
console.log(person.toJSON());
```

## API Reference

### Automatic Bindings

The generator automatically creates:

**Properties**: Direct access to C++ members
```javascript
obj.memberName = value;    // Set
console.log(obj.memberName); // Get
```

**Methods**: Direct calls to C++ methods
```javascript
obj.methodName(arg1, arg2);
```

**Getters/Setters**: Alternative property access
```javascript
obj.setMemberName(value);
const value = obj.getMemberName();
```

### Introspection API

Every bound object includes these methods:

- `getClassName()` → `string` - Get class name
- `getMemberNames()` → `Array<string>` - Get all member names  
- `getMethodNames()` → `Array<string>` - Get all method names
- `hasMember(name)` → `boolean` - Check if member exists
- `hasMethod(name)` → `boolean` - Check if method exists
- `getMemberValue(name)` → `any` - Get member value by name
- `setMemberValue(name, value)` → `void` - Set member value by name
- `callMethod(name, args)` → `any` - Call method by name
- `toJSON()` → `string` - Export object to JSON

### Module Utilities

The module provides:

- `getAllClasses()` → `Array<string>` - Get all bound class names
- `createClassName()` → `Object` - Factory function for each class

## Supported Types

### C++ to JavaScript
- `std::string` ↔ `string`
- `int` ↔ `number`
- `double`, `float` ↔ `number`
- `bool` ↔ `boolean`
- `void` ↔ `undefined`
- `vector<int>` ↔ `Array<number>`
- `vector<float or double>` ↔ `Array<number>`
- `vector<string>` ↔ `Array<string>`
- User defined...

### Type Conversion

The generator handles automatic type conversion:

```javascript
// JavaScript → C++
person.age = 30;           // number → int
person.name = "Alice";     // string → std::string
person.isActive = true;    // boolean → bool

// C++ → JavaScript  
const age = person.age;    // int → number
const name = person.name;  // std::string → string
```

## Error Handling

```javascript
try {
    person.nonExistentMember = "value";
} catch (error) {
    console.log(error.message); // "Member not found: nonExistentMember"
}

try {
    person.methodWithWrongArgs();
} catch (error) {
    console.log(error.message); // "Method 'method' expects 2 arguments, got 0"
}
```

## Performance

The binding generator optimizes for:
- **Fast Property Access**: Direct member access without string lookups
- **Efficient Method Calls**: Minimal overhead for method invocation
- **Type Conversion**: Optimized conversion between JavaScript and C++ types
- **Memory Management**: Automatic cleanup using shared_ptr

## Comparison with Manual Bindings

### Manual N-API Code (100+ lines)
```cpp
// Lots of boilerplate...
Napi::Value GetName(const Napi::CallbackInfo& info) { /* ... */ }
Napi::Value SetName(const Napi::CallbackInfo& info) { /* ... */ }
Napi::Value GetAge(const Napi::CallbackInfo& info) { /* ... */ }
// ... repeat for every member and method
```

### Automatic Bindings (1 line)
```cpp
JavascriptBindingGenerator(env, exports).bind_classes<Person, Vehicle>();
```

## Extending the Generator

To add support for custom types:

```cpp
// Register custom type at runtime
generator.register_type_converter(
    "Vector3D",
    // C++ to JS
    [](Napi::Env env, const std::any &value) -> Napi::Value {
        auto vec = std::any_cast<Vector3D>(value);
        auto obj = Napi::Object::New(env);
        obj.Set("x", vec.x);
        obj.Set("y", vec.y);
        obj.Set("z", vec.z);
        return obj;
    },
    // JS to C++
    [](const Napi::Value &js_val) -> std::any {
        auto obj = js_val.As<Napi::Object>();
        return Vector3D(obj.Get("x").As<Napi::Number>().FloatValue(),
                        obj.Get("y").As<Napi::Number>().FloatValue(),
                        obj.Get("z").As<Napi::Number>().FloatValue());
    });
```

## Troubleshooting

### Build Issues
```bash
# Clean and rebuild
npm run clean
npm install
npm run build
```

### Node.js Version Issues
```bash
# Check version
node --version  # Should be 14+
npm --version
```

### Missing Dependencies
```bash
# Install build tools (Ubuntu/Debian)
sudo apt-get install build-essential python3

# Install build tools (macOS)
xcode-select --install

# Install build tools (Windows)
npm install --global windows-build-tools
```

## Examples

See `test.js` for comprehensive usage examples including:
- Basic property and method access
- Error handling
- Dynamic introspection
- Performance testing
- Advanced scenarios

The automatic JavaScript binding generator provides the same ease of use as the Python version - minimal code to expose complete C++ classes to JavaScript with full type safety and natural language integration.

# Key Issues Fixed

- **Include paths** - Updated binding.gyp to use correct relative paths
- **RTTI** support - Enabled -frtti flag for typeid usage
macOS deployment target - Updated to 10.14 for std::any support
- **C++20** features - Ensured C++20 is properly enabled for starts_with()
- **N-API** signatures - Fixed lambda captures and property accessor creation
- **TypeInfo** copying - Made it non-copyable and used references appropriately

## License

LGPL License - see LICENSE file for details
