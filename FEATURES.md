# Rosetta Introspection System - Feature List

## Core Introspection Features

### 📦 Class Registration
- **Non-intrusive registration** - No modification of original classes required
- **Runtime metadata** - Complete class information available at runtime
- **Type-safe operations** - Strong typing with automatic conversions
- **Chaining API** - Fluent interface for registration

### 🏗️ Constructors
- **Default constructors** - Zero-argument constructors
- **Parameterized constructors** - Multiple constructors with different signatures
- **Constructor metadata** - Arity and parameter types stored
- **Runtime instantiation** - Create objects from metadata
- **Type validation** - Automatic type checking and conversion for constructor arguments

### 🔧 Fields (Member Variables)
- **Direct field access** - Register public member variables
- **Property-based access** - Virtual fields using getter/setter methods
- **Read-only properties** - Getter without setter
- **Write-only properties** - Setter without getter
- **Auto-property detection** - Automatically detect `get*/set*` method pairs
- **Base class fields** - Access fields from parent classes
- **Type information** - Store and retrieve field types

### 🎯 Methods

#### Instance Methods
- **Non-const methods** - Regular member functions
- **Const methods** - Methods that don't modify object state
- **Method overloading** - Multiple methods with same name (different signatures)
- **Variadic arguments** - Support for any number of parameters
- **Return type tracking** - Store return type information
- **Argument type tracking** - Store parameter types
- **Base class methods** - Register methods from parent classes

#### Static Methods
- **Static method registration** - Register static member functions
- **No-object invocation** - Call static methods without object instance via `invoke_static_method()`
- **Backward compatibility** - Can also call via `invoke_method()` with dummy object
- **Static method identification** - Check if method is static via `is_static_method()`

#### Virtual Methods
- **Virtual method registration** - Mark methods as virtual
- **Pure virtual methods** - Abstract method declarations
- **Override tracking** - Track method overrides in derived classes
- **Virtual table (vtable) info** - Store virtual method information

### 🧬 Inheritance

#### Inheritance Types
- **Single inheritance** - One base class
- **Multiple inheritance** - Multiple base classes
- **Virtual inheritance** - Diamond inheritance support
- **Access specifiers** - Public, protected, private inheritance

#### Inheritance Information
- **Base class list** - Track all base classes
- **Virtual base list** - Separate tracking for virtual bases
- **Inheritance type** - Normal vs virtual inheritance
- **Base class offsets** - Memory layout information
- **Polymorphism detection** - Automatic detection of polymorphic classes
- **Abstract class detection** - Identify abstract classes
- **Virtual destructor detection** - Check for virtual destructors

### Functors / Function Objects
* ✅ **std::function parameters** - Methods can accept `std::function<Ret(Args...)>` as parameters
* ⚠️ **Functor classes** - Can register `operator()` as a regular method, but no special functor detection
* 🚧 **std::function parameter detection** - Infrastructure exists in `function_auto_registration.h` but not yet integrated (commented out in method registration)
* ❌ **Lambda introspection** - Cannot introspect lambda types directly (must convert to `std::function`)
* ❌ **Automatic callable detection** - No automatic detection of types with `operator()` (must manually register)

📝 **Notes:** 
- `std::function` can be stored in `Any` and passed as method parameters
- The function's implementation is opaque (signature visible via type system, but body cannot be introspected)
- Lambda support requires explicit conversion: `std::function<Ret(Args...)> f = [](Args...){ ... }`

### Free Functions
- **Global functions** - Can register standalone functions
- **Namespace-level functions** - Support for non-member functions


### 🎭 Type System

#### Primitive Types
- ✅ `void`
- ✅ `bool`
- ✅ `int`, `long`, `long long`
- ✅ `float`, `double`
- ✅ `size_t`
- ✅ `std::string`

#### Container Types
- ✅ `std::vector<T>`
- ✅ `std::map<K, V>`
- ✅ `std::unordered_map<K, V>`
- ✅ `std::set<T>`
- ✅ `std::unordered_set<T>`
- ✅ `std::array<T, N>`
- ✅ `std::deque<T>`
- ✅ `std::optional<T>` (detection only)

#### Pointer Types
- ✅ Raw pointers `T*`
- ✅ `std::shared_ptr<T>`
- ✅ `std::unique_ptr<T>`
- ✅ `std::weak_ptr<T>`
- ✅ References `T&`, `T&&`

#### Custom Types
- ✅ User-defined classes
- ✅ Nested classes
- ✅ Template classes

### 🔄 Type Conversions
- **Automatic numeric conversions** - `int` ↔ `double` ↔ `float`
- **Type-safe casting** - Runtime type checking
- **Any wrapper** - Type-erased value container
- **Pointer trait detection** - Automatic pointer type recognition
- **Container trait detection** - Automatic container type recognition

### 📝 Type Information

#### Type Naming
- **Name demangling** - Convert mangled names to readable format (GCC/Clang)
- **Type cleanup** - Remove implementation details (`std::__1::`, `std::__cxx11::`)
- **Allocator removal** - Clean allocator template parameters
- **Custom type aliases** - Register readable names for types
- **Type registry** - Central registry for type name mappings

#### Type Traits
- **Container traits** - Detect and analyze STL containers
- **Pointer traits** - Detect and analyze pointers/references
- **Inheritance traits** - Detect polymorphism, abstract classes
- **Type kind detection** - Categorize types (primitive, container, pointer, object)

---

## ❌ Currently NOT Supported

### Templates
- ❌ **Template introspection** - Cannot query template parameters at runtime
- ❌ **Concept constraints** - No C++20 concept support
- ⚠️ **Note**: Can register template *instantiations* (e.g., `Vector<int>`, `Vector<double>`)

### Enumerations
- ❌ **Enum registration** - No built-in enum support
- ❌ **Enum to string** - No automatic enum value naming
- ❌ **Scoped enums (enum class)** - Not supported
- ⚠️ **Workaround**: Register as integer fields or use third-party enum libraries

### Operators
- ❌ **Operator overloading** - No special syntax for `operator+`, `operator[]`, etc.
- ⚠️ **Workaround**: Register with string names like `"operator_plus"`

### Attributes / Annotations
- ❌ **C++ attributes** - Cannot read `[[nodiscard]]`, `[[deprecated]]`, etc.
- ❌ **Custom attributes** - No user-defined attribute system
- ⚠️ **Workaround**: Use documentation strings in extensions

### Advanced C++ Features
- ❌ **Variadic templates** - No introspection of parameter packs
- ❌ **SFINAE/Concepts** - No compile-time constraint introspection
- ❌ **Constexpr functions** - Cannot mark or detect constexpr
- ❌ **Coroutines** - No C++20 coroutine support

---

## 🔌 Extensions & Generators

### Documentation Generation
- ✅ **Markdown output** - Generate .md documentation
- ✅ **HTML output** - Generate HTML documentation
- ✅ **JSON output** - Machine-readable format
- ✅ **Custom formats** - Extensible format system

### Serialization
- ✅ **JSON serialization** - Serialize objects to/from JSON
- ✅ **XML serialization** - Serialize objects to/from XML
- ✅ **Automatic serialization** - No manual code needed
- ✅ **Nested object support** - Handle complex object graphs

### Validation
- ✅ **Constraint system** - Define validation rules
- ✅ **Range constraints** - Min/max validation
- ✅ **Size constraints** - Container size validation
- ✅ **Not-null constraints** - Pointer validation
- ✅ **Custom constraints** - User-defined validation logic

### Language Bindings

#### Python (via pybind11)
- ✅ **Automatic binding generation** - No manual pybind11 code
- ✅ **Constructor binding** - All constructors exposed
- ✅ **Field binding** - Properties in Python
- ✅ **Method binding** - All methods callable from Python
- ✅ **Container conversion** - Automatic list/dict/set conversion
- ✅ **Type conversion registry** - Custom type converters
- ✅ **Static method support** - Python static methods

#### JavaScript/TypeScript (planned)
- 🚧 **Node.js bindings** - Via N-API or node-addon-api
- 🚧 **TypeScript definitions** - Automatic .d.ts generation
- 🚧 **WebAssembly support** - Compile C++ to WASM with bindings

---

## 🎯 Use Cases

### What Rosetta IS Good For:
✅ **Scripting language bindings** - Python, JavaScript, Lua  
✅ **Serialization systems** - Save/load object graphs  
✅ **Property editors** - GUI property panels  
✅ **RPC/IPC systems** - Remote procedure calls  
✅ **Configuration systems** - Load from JSON/XML/YAML  
✅ **Plugin systems** - Load classes dynamically  
✅ **Testing frameworks** - Inspect object state  
✅ **Data binding** - UI to data model binding  
✅ **Object cloning** - Deep copy via metadata  

### What Rosetta is NOT:
❌ **Replacement for templates** - Use templates for compile-time polymorphism  
❌ **Serialization for hot paths** - Use hand-written serializers for performance  
❌ **Complete reflection system** - C++ doesn't support full reflection yet  
❌ **ABI stability tool** - Metadata can change between compilations  

---

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

---

## 🔮 Future Enhancements

### Planned Features
- 🔜 **Free function support** - Register global functions
- 🔜 **Enum support** - Full enum introspection
- 🔜 **Operator registration** - Special syntax for operators
- 🔜 **Attribute system** - User-defined metadata tags
- 🔜 **JavaScript bindings** - Complete JS/TS generator

### Under Consideration
- 💭 **Compile-time reflection** - C++26 reflection integration when available
- 💭 **ABI versioning** - Stable metadata format
- 💭 **Binary serialization** - Efficient binary format
- 💭 **Protobuf integration** - Generate .proto files
- 💭 **GraphQL schema generation** - Automatic API generation

---

**Version**: 1.0.0  
**Last Updated**: 2025  
**License**: LGPL3<br>
**Author**: [xaliphostes](https://github.com/xaliphostes)
