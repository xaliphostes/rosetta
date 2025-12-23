# 4. Any - Type-Erased Container

A type-erased container similar to `std::any` but with enhanced features for Rosetta, including `reference_wrapper` unwrapping, numeric conversions, and support for non-copyable types.

---

## Construction

### Basic Constructors
```cpp
Any()                                  // Default constructor (empty Any)
Any(T value)                           // Store any copyable/movable value
Any(const char* str)                   // C-string → std::string conversion
Any(const Any& other)                  // Copy constructor (deep copy)
Any(Any&& other)                       // Move constructor
```

### Storage Behavior by Argument Type
```cpp
Any a(value)                           // Lvalue → COPY into storage
Any a(std::move(value))                // Rvalue → MOVE into storage
Any a(ptr)                             // T* → stores POINTER (no copy of T)
Any a(cptr)                            // const T* → stores POINTER (no copy of T)
Any a(std::ref(obj))                   // reference_wrapper<T> → stores wrapper (no copy)
Any a(std::cref(obj))                  // reference_wrapper<const T> → stores wrapper (no copy)
Any a(shared_ptr)                      // shared_ptr<T> → copies shared_ptr (ref count++)
Any a(std::move(unique_ptr))           // unique_ptr<T> → moves unique_ptr (no copy)
```

---

## Value Access

### Typed Access
```cpp
T& as<T>()                             // Get mutable reference to stored value
const T& as<T>() const                 // Get const reference to stored value
```

**Throws:** `std::bad_cast` if type mismatch or empty

### Automatic Unwrapping
```cpp
Any a(std::ref(obj));
T& ref = a.as<T>();                    // Unwraps reference_wrapper<T> automatically
```

### Numeric Conversions
```cpp
Any a(42);                             // Stores int
double d = a.as<double>();             // Converts int → double
float f = a.as<float>();               // Converts int → float
```
Supported conversions: `int` ↔ `float` ↔ `double`

### Raw Access
```cpp
const void* get_void_ptr() const       // Raw pointer to stored value (nullptr if empty)
```

---

## Type Inspection

```cpp
bool has_value() const                 // Check if Any contains a value
bool is_copyable() const               // Check if stored type is copyable
std::string type_name() const          // Mangled type name ("empty" if no value)
std::type_index type() const           // Get std::type_index (typeid(void) if empty)
std::type_index get_type_index() const // Synonym for type()
```

---

## Modification

```cpp
Any& operator=(const Any& other)       // Copy assignment (deep copy)
Any& operator=(Any&& other)            // Move assignment
void reset()                           // Clear value (becomes empty)
```

---

## String Conversion

```cpp
std::string toString() const           // Convert to string representation
```

**Returns:**
- `"<empty>"` if no value
- Converted string if type is registered in `AnyStringRegistry`
- `"<TypeName>"` otherwise (using demangled type name)

### AnyStringRegistry

Register custom string converters for types:

```cpp
AnyStringRegistry::instance().register_type<MyType>(
    [](const MyType& v) { return v.to_string(); }
);
```

**Pre-registered types:** `int`, `float`, `double`, `bool`, `std::string`

---

## Non-Copyable Types

`Any` supports move-only types like `std::unique_ptr`:

```cpp
Any a(std::make_unique<int>(42));      // ✓ Works - moves into Any
a.is_copyable();                       // → false

Any b(a);                              // ✗ Throws std::logic_error at runtime
Any c(std::move(a));                   // ✓ Works - moves Any
```

**Best Practice:** Check `is_copyable()` before copying an `Any` of unknown contents.

---

## Type Matching Rules

| Stored Type | `as<T>()` Call | Result |
|-------------|----------------|--------|
| `T` | `as<T>()` | ✓ Direct access |
| `reference_wrapper<T>` | `as<T>()` | ✓ Unwraps automatically |
| `int` | `as<double>()` | ✓ Numeric conversion |
| `T*` | `as<T*>()` | ✓ Returns pointer |
| `T*` | `as<const T*>()` | ✗ `bad_cast` (different type!) |
| `T*` | `as<T>()` | ✗ `bad_cast` |
| `const char*` | `as<std::string>()` | ✓ (stored as string) |
| `const char*` | `as<const char*>()` | ✗ `bad_cast` |
| Empty | `as<T>()` | ✗ `bad_cast` |

---

## Complete Example

```cpp
using namespace rosetta::core;

// Value storage
Any a1(42);
Any a2(std::string("hello"));
Any a3(3.14159);

// Pointer storage (no copy of pointed-to object)
MyClass obj;
Any a4(&obj);                          // Stores MyClass*
a4.as<MyClass*>()->doSomething();      // Access through pointer

// Reference semantics (no copy)
Any a5(std::ref(obj));
a5.as<MyClass>().value = 100;          // Modifies original obj!

// Smart pointers
auto sp = std::make_shared<MyClass>();
Any a6(sp);                            // Ref count = 2

auto up = std::make_unique<MyClass>();
Any a7(std::move(up));                 // up is now nullptr
a7.is_copyable();                      // → false

// Type inspection
a1.has_value();                        // → true
a1.type() == typeid(int);              // → true
a1.toString();                         // → "42"

// Copy vs Move
Any copy = a1;                         // Deep copy
Any moved = std::move(a1);             // a1 is now empty
a1.has_value();                        // → false
```

---

## Summary Table

| Operation | Copies T? | Notes |
|-----------|-----------|-------|
| `Any(lvalue)` | Yes | Copy into storage |
| `Any(rvalue)` | No | Move into storage |
| `Any(T*)` | No | Stores pointer value only |
| `Any(std::ref(x))` | No | Stores lightweight wrapper |
| `Any(shared_ptr)` | No* | Copies shared_ptr, not T |
| `Any(unique_ptr)` | No | Must move; Any becomes non-copyable |
| `Any copy(other)` | Yes | Deep copy of stored value |
| `Any moved(std::move(other))` | No | Transfers ownership |