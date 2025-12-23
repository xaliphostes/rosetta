# Rosetta introspection doc

## 1. Registration API (Fluent Builder Pattern)

Fluent builder pattern is a style of coding which force the developer to create the object in sequence by calling each setter method one after the another until all required attributes are set.

### Constructors
```cpp
.constructor<>()                                       // Default constructor
.constructor<Args...>()                                // Parameterized constructors (multiple allowed)
```

### Fields (Direct Member Access)
```cpp
.field("name", &Class::member)                         // Public member variable
.base_field<Base>("name", &Base::member)               // Field from base class
```

### Properties (Virtual Fields via Getter/Setter)
```cpp
// Multiple signature variants supported:
.property("name", &Class::getter, &Class::setter)      // const T& (Class::*)() const, void (Class::*)(const T&)
.property("name", &Class::getter, &Class::setter)      // T& (Class::*)(), void (Class::*)(const T&)
.property("name", &Class::getter, &Class::setter)      // T (Class::*)() const, void (Class::*)(T)
.property<T>("name", &Class::getter, &Class::setter)   // Explicit type specification

.readonly_property("name", &Class::getter)             // Read-only property
.writeonly_property("name", &Class::setter)            // Write-only property

.auto_detect_properties()                              // Auto-detect from get*/set* pairs
```

### Methods
```cpp
.method("name", &Class::method)                        // Non-const method
.method("name", &Class::method)                        // Const method (auto-detected)
.static_method("name", &Class::staticMethod)           // Static method
```

### Method Overloads (Disambiguation Helpers)
```cpp
.method("name", rosetta::core::overload<Ret, Args...>(&Class::method))
.method("name", rosetta::core::overload_const<Ret, Args...>(&Class::method))
.method("name", rosetta::core::overload_static<Ret, Args...>(&Class::staticMethod))

// Or via macros:
.method("name", ROSETTA_OVERLOAD(Class, RetType, methodName, ArgTypes...))
.method("name", ROSETTA_OVERLOAD_CONST(Class, RetType, methodName, ArgTypes...))
.method("name", ROSETTA_OVERLOAD_STATIC(Class, RetType, methodName, ArgTypes...))
```

### Virtual Methods & Inheritance
```cpp
.inherits_from<Base>("BaseName")                       // Normal inheritance
.virtually_inherits_from<Base>("BaseName")             // Virtual inheritance

.virtual_method("name", &Class::method)                // Virtual method
.pure_virtual_method<RetType>("name")                  // Pure virtual (abstract)
.override_method("name", &Class::method)               // Override of base method
```

### Synthetic Methods (Lambda/Callable as Methods)
```cpp
.lambda_method<Ret, Args...>("name", [](Class& self, Args...) { ... })
.lambda_method_const<Ret, Args...>("name", [](const Class& self, Args...) { ... })
```

### Free Functions
```cpp
ROSETTA_REGISTER_FUNCTION(function_name)
ROSETTA_REGISTER_FUNCTION_AS(function_name, "customName")
```

## 2. Runtime Introspection API (Query & Invoke)

### Constructors
```cpp
meta.constructors()                    // std::vector<Constructor> - all registered constructors
meta.constructor_infos()               // std::vector<ConstructorInfo> - with param types & arity
meta.construct({arg1, arg2, ...})      // Invoke constructor dynamically → Any
```

### Fields / Properties
```cpp
meta.fields()                          // std::vector<std::string> - all field names
meta.get_field(obj, "name")            // Get field value → Any
meta.set_field(obj, "name", value)     // Set field value
meta.get_field_type("name")            // std::type_index of field type
meta.has_field("name")                 // bool - check if field exists
```

### Methods
```cpp
meta.methods()                         // std::vector<std::string> - all method names
meta.method_info("name")               // std::vector<MethodInfo> - overload info (arity, arg_types, return_type, is_static)
meta.invoke_method(obj, "name", {args})          // Invoke non-const method → Any
meta.invoke_const_method(obj, "name", {args})    // Invoke const method → Any
meta.has_method("name")                          // bool - check if method exists
```

### Inheritance Info
```cpp
meta.inheritance()                     // InheritanceInfo struct with:
    .base_classes                      //   - std::vector<BaseInfo>
    .virtual_bases                     //   - std::vector<BaseInfo> (virtual inheritance)
    .is_abstract                       //   - bool
    .is_polymorphic                    //   - bool
    .has_virtual_destructor            //   - bool
    .vtable                            //   - VirtualTable (virtual method registry)
```

### Debug / Dump
```cpp
meta.name()                            // Class name as string
meta.dump(std::ostream&)               // Pretty-print all metadata
```

### Global Registry Access
```cpp
ROSETTA_HAS_CLASS(ClassName)           // Check if class is registered
ROSETTA_GET_META(ClassName)            // Get ClassMetadata<T>&

Registry::instance().get_by_name("ClassName")    // Get type-erased holder by name
Registry::instance().has_class<T>()              // Check registration
Registry::instance().get<T>()                    // Get typed metadata
```

## 3. Supported Member Types

- Primitives: `bool`, `int`, `double`, `float`, `uint32_t`, `std::string`
- STL Containers: `std::vector`, `std::array`, `std::map`, `std::set`
- Smart Pointers: `std::shared_ptr`, `std::unique_ptr`, raw pointers
- Functors: `std::function<Ret(Args...)>` as fields
- Nested Objects: Any Rosetta-registered class
