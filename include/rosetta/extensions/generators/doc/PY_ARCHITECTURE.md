# Unified Python Binding Architecture

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     PythonBinder (Main API)                     │
│  - bind_class<T>() [manual]                                     │
│  - bind_all_registered() [automatic]                            │
│  - bind_function(), bind_constant()                             │
│  - add_utilities()                                              │
└────────────┬───────────────────────────────┬────────────────────┘
             │                               │
             ▼                               ▼
┌────────────────────────┐    ┌──────────────────────────────────┐
│   ClassBinder<T>       │    │    BinderRegistry                │
│   (Shared Logic)       │    │    (Type Erasure)                │
│                        │    │                                  │
│ - bind()               │◄───┤ - register_class<T>()            │
│ - bind_constructors()  │    │ - bind_all()                     │
│ - bind_fields()        │    │                                  │
│ - bind_methods()       │    │  TypeErasedBinder                │
│ - bind_static_methods()│    │    │                             │
└───────┬────────────────┘    │    └─► TypedBinder<T>            │
        │                     │          └─► ClassBinder<T>      │
        │                     └──────────────────────────────────┘
        │
        ▼
┌────────────────────────────────────────────────────────────────┐
│                    TypeConverter                               │
│                   (Zero Duplication)                           │
│                                                                │
│  - any_to_python(Any) → py::object                             │
│  - python_to_any(py::object, type_index) → Any                 │
│  - python_to_any(py::object) → Any  [auto-detect]              │
│  - register_type<T>()  [extensible]                            │
│                                                                │
│  ConverterRegistry:                                            │
│    - to_cpp:    type_index → (py::object → Any)                │
│    - to_python: type_index → (Any → py::object)                │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────────┐
│                  Rosetta Core Registry                         │
│  - ClassMetadata<T>                                            │
│  - Fields, Methods, Constructors                               │
│  - Type information                                            │
└────────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

### 1. PythonBinder (Façade)
**Purpose**: Main API for users
**Responsibilities**:
- Provide simple interface for binding
- Support both manual and automatic workflows
- Delegate to appropriate components

```cpp
PythonBinder binder(module);
binder.bind_class<T>()           // → ClassBinder<T>::bind()
      .bind_all_registered()     // → BinderRegistry::bind_all()
      .add_utilities();
```

### 2. ClassBinder<T> (Worker)
**Purpose**: Actually perform the binding for a class
**Responsibilities**:
- Get metadata from Rosetta Registry
- Create pybind11 class
- Bind constructors, fields, methods
- **Used by BOTH manual and automatic binding** ← Key!

```cpp
// Used by manual binding:
binder.bind_class<MyClass>() 
  → ClassBinder<MyClass>::bind()

// Used by automatic binding:
BinderRegistry::bind_all()
  → TypedBinder<MyClass>::bind() 
    → ClassBinder<MyClass>::bind()
```

### 3. BinderRegistry (Type Erasure)
**Purpose**: Store type information for automatic binding
**Responsibilities**:
- Store `TypedBinder<T>` for each registered class
- Allow runtime iteration without knowing types
- Enable `bind_all()` functionality

```cpp
// At registration time (compile-time):
ROSETTA_REGISTER_CLASS_PY(MyClass)
  → BinderRegistry::register_class<MyClass>("MyClass")
    → Store TypedBinder<MyClass>

// At binding time (runtime):
binder.bind_all_registered()
  → for each TypedBinder: binder->bind(module)
    → ClassBinder<T>::bind(module)
```

### 4. TypeConverter (Utility)
**Purpose**: Convert between C++ and Python types
**Responsibilities**:
- Handle primitive types
- Handle STL containers
- Support custom types via registry
- **Single implementation used everywhere** ← Key!

```cpp
// In ClassBinder::bind_fields():
py::object py_val = TypeConverter::any_to_python(cpp_val);

// In ClassBinder::bind_methods():
core::Any cpp_arg = TypeConverter::python_to_any(py_arg, expected_type);

// Extensible:
TypeConverter::register_type<MyCustomType>();
```

## Data Flow Examples

### Manual Binding Flow

```
User Code:
  binder.bind_class<Person>()
         ↓
PythonBinder::bind_class<Person>()
         ↓
ClassBinder<Person>::bind(module, "Person")
         ├─→ Get Registry::get<Person>() metadata
         ├─→ Create py::class_<Person>
         ├─→ bind_constructors() ──→ TypeConverter
         ├─→ bind_fields()       ──→ TypeConverter
         ├─→ bind_methods()      ──→ TypeConverter
         └─→ bind_static_methods() → TypeConverter
```

### Automatic Binding Flow

```
User Code:
  binder.bind_all_registered()
         ↓
PythonBinder::bind_all_registered()
         ↓
BinderRegistry::bind_all(module)
         ├─→ For "Person": TypedBinder<Person>::bind()
         │                       ↓
         │              ClassBinder<Person>::bind() ──┐
         │                                            │
         ├─→ For "Vehicle": TypedBinder<Vehicle>::bind()
         │                       ↓                    │
         │              ClassBinder<Vehicle>::bind()──┤
         │                                            │
         └─→ ... more classes ...                     │
                                                      │
         ┌────────────────────────────────────────────┘
         ↓
    [Same binding logic as manual!]
    Uses TypeConverter for all conversions
```

## Registration Flow

```
C++ Code:
  ROSETTA_REGISTER_CLASS_PY(Person)
    .field("name", &Person::name)
    .method("greet", &Person::greet);

Expands to:
  1. Registry::register_class<Person>("Person")
     → Store ClassMetadata<Person>
     
  2. BinderRegistry::register_class<Person>("Person")
     → Store TypedBinder<Person>
     
  3. TypeConverter::register_type<Person>()
     → Store Person converters

Result:
  ✓ Class registered with Rosetta
  ✓ Class can be manually bound: binder.bind_class<Person>()
  ✓ Class can be auto-bound: binder.bind_all_registered()
  ✓ Person objects convert to/from Python
```

## Key Design Patterns

### 1. Template Method Pattern
`ClassBinder<T>::bind()` uses template method:
- Same sequence for all classes
- Ensures consistency
- Easy to modify globally

### 2. Type Erasure Pattern
`BinderRegistry` uses type erasure:
- `TypeErasedBinder` interface
- `TypedBinder<T>` implementation
- Allows runtime iteration over compile-time types

### 3. Registry Pattern
Both `TypeConverter` and `BinderRegistry` use registry:
- Singleton instance
- Type-indexed storage
- Runtime lookup

### 4. Façade Pattern
`PythonBinder` is a façade:
- Simplifies complex subsystem
- Provides unified interface
- Delegates to specialists

## Comparison: Old vs New

### Old Architecture (Duplicated)

```
py_generator:                python_binding_generator:
┌──────────────┐            ┌──────────────────────┐
│ PyGenerator  │            │ PythonBinding-       │
│              │            │ Generator            │
│ - bind_class │            │ - bind_all_classes   │
│              │            │ - bind_class         │
└──────┬───────┘            └──────┬───────────────┘
       │                           │
       ▼                           ▼
┌──────────────┐            ┌──────────────────────┐
│ PyClassBinder│            │ ClassBinder          │
│ <T>          │            │ methods              │
│              │            │                      │
│ 500 lines of │            │ 500 lines of         │
│ SAME CODE    │            │ SAME CODE            │
└──────────────┘            └──────────────────────┘
       │                           │
       ▼                           ▼
┌──────────────┐            ┌──────────────────────┐
│ any_to_python│            │ any_to_python        │
│ python_to_any│            │ python_to_any        │
│              │            │                      │
│ 200 lines of │            │ 200 lines of         │
│ SAME CODE    │            │ SAME CODE            │
└──────────────┘            └──────────────────────┘

Total: ~1763 lines (500+ duplicated)
```

### New Architecture (Unified)

```
        PythonBinder (180 lines)
               │
       ┌───────┴───────┐
       │               │
       ▼               ▼
ClassBinder<T>   BinderRegistry
  (300 lines)      (50 lines)
       │
       ▼
  TypeConverter
    (200 lines)
    
Total: ~680 lines (0 duplicated)
Reduction: 61%
```

## Usage Patterns

### Pattern 1: Pure Manual Binding
```cpp
ROSETTA_PY_MODULE(mymodule, "My module") {
    BIND_CLASS(Person);
    BIND_CLASS(Vehicle);
    BIND_FUNCTION(process, "Process data");
}
ROSETTA_PY_MODULE_END
```

### Pattern 2: Pure Automatic Binding
```cpp
ROSETTA_PY_MODULE(mymodule, "Auto-generated") {
    BIND_ALL_CLASSES();
}
ROSETTA_PY_MODULE_END
```

### Pattern 3: Filtered Automatic Binding
```cpp
ROSETTA_PY_MODULE(mymodule, "Filtered") {
    binder.bind_all_registered([](const std::string& name) {
        return !name.starts_with("Internal");
    });
}
ROSETTA_PY_MODULE_END
```

### Pattern 4: Hybrid Approach (Best!)
```cpp
ROSETTA_PY_MODULE(mymodule, "Hybrid") {
    // Auto-bind most classes
    binder.bind_all_registered([](const std::string& name) {
        return name != "SpecialClass";  // Exclude one
    });
    
    // Manually bind the special one with custom name
    binder.bind_class<SpecialClass>("Special");
    
    // Add extras
    BIND_FUNCTION(utility_func, "Docs");
    BIND_UTILITIES();
}
ROSETTA_PY_MODULE_END
```

## Extension Points

### 1. Custom Type Conversion
```cpp
struct MyCustomType { ... };

// Register converter
TypeConverter::register_type<MyCustomType>();

// Now works automatically in all bindings!
ROSETTA_REGISTER_CLASS_PY(MyClass)
    .field("custom", &MyClass::custom);  // MyCustomType field
```

### 2. Custom Binding Logic
```cpp
// For special cases, directly use ClassBinder
auto py_class = ClassBinder<SpecialClass>::bind(module, "Special");

// Then customize further
py_class.def("custom_method", [](SpecialClass& obj) {
    // Custom implementation
});
```

### 3. Post-Processing Hook
```cpp
// After automatic binding, access classes
binder.bind_all_registered();

// Then add module-level functionality
module.def("create_person", []() { return Person(); });
```

## Summary

The unified architecture achieves:

✅ **Zero Duplication** - Single implementation of conversion & binding logic  
✅ **Flexibility** - Support manual, automatic, or hybrid approaches  
✅ **Consistency** - Same logic regardless of binding method  
✅ **Extensibility** - Easy to add custom types and converters  
✅ **Maintainability** - 61% less code, single source of truth  
✅ **Backward Compatibility** - Drop-in replacement with macros  

**Recommended Action**: Replace old files with unified implementation.