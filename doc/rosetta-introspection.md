# Rosetta Introspection Documentation

## 1. Registration API (Fluent Builder Pattern)

Fluent builder pattern is a style of coding which forces the developer to create the object in sequence by calling each setter method one after another until all required attributes are set.

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

### Method Overloads (Disambiguation)

When a class has multiple overloaded methods with the same name, use disambiguation helpers:

```cpp
// Using overloaded_method() with helper functions:
.overloaded_method("name", rosetta::core::overload<Ret, Class, Args...>(&Class::method))
.overloaded_method("name", rosetta::core::overload_const<Ret, Class, Args...>(&Class::method))

// Using overloaded_method() with macros (shorter syntax):
.overloaded_method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, int, int))
.overloaded_method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, const String&, const String&))
.overloaded_method("getValue", ROSETTA_OVERLOAD_CONST(MyClass, double, getValue))
.overloaded_method("getValue", ROSETTA_OVERLOAD_CONST(MyClass, double, getValue, int))

// Static method overloads:
.static_method("create", rosetta::core::overload_static<MyClass*, int>(&MyClass::create))
.static_method("create", ROSETTA_OVERLOAD_STATIC(MyClass, MyClass*, create, const String&))
```

**Available Helper Functions:**
```cpp
rosetta::core::overload<Ret, Class, Args...>(ptr)        // Non-const member function
rosetta::core::overload_const<Ret, Class, Args...>(ptr)  // Const member function
rosetta::core::overload_static<Ret, Args...>(ptr)        // Static function

// Short aliases:
rosetta::core::sel<Ret, Class, Args...>(ptr)             // Alias for overload
rosetta::core::sel_const<Ret, Class, Args...>(ptr)       // Alias for overload_const
```

**Available Macros:**
```cpp
ROSETTA_OVERLOAD(Class, RetType, methodName, ArgTypes...)
ROSETTA_OVERLOAD_CONST(Class, RetType, methodName, ArgTypes...)
ROSETTA_OVERLOAD_STATIC(Class, RetType, methodName, ArgTypes...)

// Short aliases:
R_OVERLOAD(Class, RetType, methodName, ArgTypes...)
R_OVERLOAD_CONST(Class, RetType, methodName, ArgTypes...)
R_OVERLOAD_STATIC(Class, RetType, methodName, ArgTypes...)
```

### Virtual Methods & Inheritance

```cpp
// Declare inheritance (template parameter can include namespace)
.inherits_from<Base>("BaseName")                           // Normal inheritance
.inherits_from<arch::IterativeSolver>("IterativeSolver")   // With namespace
.virtually_inherits_from<Base>("BaseName")                 // Virtual inheritance

// Register base class methods
.base_method<Base>("name", &Base::method)                  // Non-const base method
.base_method<Base>("name", &Base::constMethod)             // Const base method (auto-detected)

// Virtual method registration
.virtual_method("name", &Class::method)                    // Virtual method
.pure_virtual_method<Ret, Args...>("name")                 // Pure virtual (non-const)
.pure_virtual_method_const<Ret, Args...>("name")           // Pure virtual (const)
.override_method("name", &Class::method)                   // Override of base method
```

### Synthetic Methods (Lambda/Callable as Methods)

Register lambdas or callables as if they were class methods. The first parameter of the lambda receives the object instance (`self`).

```cpp
// Simple syntax - only specify return type (Args deduced from lambda)
.lambda_method<double>("x", [](const arch::Vector3 &v) { return v[0]; })
.lambda_method<double>("y", [](const arch::Vector3 &v) { return v[1]; })
.lambda_method<double>("z", [](const arch::Vector3 &v) { return v[2]; })

// String representation for Python __repr__
.lambda_method<std::string>("__repr__", [](const arch::Vector3 &v) {
    return "Vector3(" + std::to_string(v[0]) + ", " + 
                        std::to_string(v[1]) + ", " + 
                        std::to_string(v[2]) + ")";
})

// Full syntax - specify return type and argument types
.lambda_method<void, double, int>("runCustom", [](Solver& self, double tol, int maxiter) {
    self.setTolerance(tol);
    self.setMaxIterations(maxiter);
    self.run();
})

// Const version (first parameter is const Class&)
.lambda_method_const<double>("magnitude", [](const Vector3& self) {
    return std::sqrt(self.x*self.x + self.y*self.y + self.z*self.z);
})

// With arguments
.lambda_method_const<double, int>("getComponent", [](const Vector3& self, int index) {
    return self[index];
})
```

**Use Cases for Lambda Methods:**
- Adding Python special methods (`__repr__`, `__str__`, `__len__`, `__getitem__`)
- Creating convenience wrappers around complex method calls
- Adding computed properties that don't exist in the C++ class
- Adapting method signatures for target language conventions

### Free Functions

```cpp
ROSETTA_REGISTER_FUNCTION(function_name)
ROSETTA_REGISTER_FUNCTION_AS(function_name, "customName")
```

---

## 2. Runtime Introspection API (Query & Invoke)

### Class Metadata Access

```cpp
// Via macros
ROSETTA_HAS_CLASS(ClassName)                 // Check if class is registered → bool
ROSETTA_GET_META(ClassName)                  // Get ClassMetadata<T>&

// Via Registry singleton
auto& registry = rosetta::core::Registry::instance();
registry.has_class<T>()                      // Check registration           → bool
registry.has_class("ClassName")              // Check by name                → bool
registry.get<T>()                            // Get typed metadata           → ClassMetadata<T>&
registry.get_by_name("ClassName")            // Get type-erased holder       → MetadataHolder*
registry.list_classes()                      // List all registered classes  → std::vector<std::string>
registry.size()                              // Number of registered classes → size_t
registry.clear()                             // Unregister all classes
```

### Constructors

```cpp
meta.constructors()                          // All registered constructors → std::vector<Constructor>
meta.constructor_infos()                     // Detailed info               → std::vector<ConstructorInfo>
                                             //   ConstructorInfo contains:
                                             //     .param_types         → std::vector<std::type_index>
                                             //     .param_is_lvalue_ref → std::vector<bool>
                                             //     .arity               → size_t
meta.is_instantiable()                       // Can create instances?    → bool
```

### Fields (Direct Member Variables)

```cpp
meta.fields()                                // All field names → std::vector<std::string>
meta.get_field(obj, "name")                  // Get field value → Any
meta.set_field(obj, "name", value)           // Set field value
meta.get_field_type("name")                  // Field type → std::type_index
```

### Properties (Virtual Fields via Getter/Setter)

```cpp
meta.properties()                            // All property names → std::vector<std::string>
meta.is_property("name")                     // Check if property exists → bool
meta.get_property(obj, "name")               // Get via getter → Any
meta.set_property(obj, "name", value)        // Set via setter
meta.get_property_type("name")               // Property type → std::type_index
meta.get_property_info("name")               // Detailed info → PropertyInfo
                                             //   PropertyInfo contains:
                                             //     .name             → std::string
                                             //     .getter_name      → std::string
                                             //     .setter_name      → std::string
                                             //     .value_type       → std::type_index
                                             //     .is_readonly      → bool
                                             //     .is_writeonly     → bool
```

### Methods

```cpp
meta.methods()                               // All method names → std::vector<std::string>
meta.method_info("name")                     // All overloads → std::vector<MethodInfo>
                                             //   MethodInfo contains:
                                             //     .arg_types        → std::vector<std::type_index>
                                             //     .return_type      → std::type_index
                                             //     .arity            → size_t
                                             //     .is_const         → bool
                                             //     .is_static        → bool
                                             //     .is_overloaded    → bool
                                             //     .is_lambda        → bool
                                             //     .inherited_from   → std::string (empty if not inherited)

meta.is_method_overloaded("name")            // Has multiple overloads? → bool
meta.is_method_const("name")                 // Is const method?        → bool
meta.is_static_method("name")                // Is static method?       → bool

// Invoke methods (automatic overload resolution based on argument types)
meta.invoke_method(obj, "name", {args})      // Invoke on mutable object → Any
meta.invoke_method(constObj, "name", {args}) // Invoke on const object   → Any
meta.invoke_method(ptr, "name", {args})      // Invoke on pointer (raw, shared, unique) → Any
meta.invoke_static_method("name", {args})    // Invoke static method     → Any

// Query method signatures (returns first overload for backward compatibility)
meta.get_method_arity("name")                // Number of parameters → size_t
meta.get_method_arg_types("name")            // Parameter types      → std::vector<std::type_index>
meta.get_method_return_type("name")          // Return type          → std::type_index

// Query all overloads
meta.get_method_arities("name")              // Arities of all overloads      → std::vector<size_t>
meta.get_method_arg_types_all("name")        // Arg types of all overloads    → std::vector<std::vector<std::type_index>>
meta.get_method_return_types("name")         // Return types of all overloads → std::vector<std::type_index>
```

### Inheritance Info

```cpp
meta.inheritance()                           // Get inheritance info → InheritanceInfo&
meta.name()                                  // Class name           → std::string

// InheritanceInfo structure:
inheritance.base_classes                     // Direct bases                → std::vector<BaseClassInfo>
inheritance.virtual_bases                    // Virtual bases               → std::vector<BaseClassInfo>
inheritance.is_abstract                      // Has pure virtuals?          → bool
inheritance.is_polymorphic                   // Has virtual methods?        → bool
inheritance.has_virtual_destructor           // Virtual destructor?         → bool
inheritance.vtable                           // Virtual table info          → VirtualTableInfo
inheritance.has_base(typeid(Base))           // Check if inherits from type → bool
inheritance.get_base(typeid(Base))           // Get base info               → BaseClassInfo*
inheritance.total_base_count()               // Total number of bases       → size_t

// BaseClassInfo structure:
base.name                                    // Base class name          → std::string
base.type                                    // Type info                → const std::type_info*
base.inheritance_type                        // Normal or Virtual        → InheritanceType
base.access                                  // Public/Protected/Private → AccessSpecifier
base.offset                                  // Memory offset            → size_t
base.is_primary_base                         // Is primary base?         → bool
```

### Debug / Dump

```cpp
meta.name()                                  // Class name → std::string
meta.dump(std::ostream&)                     // Pretty-print all metadata to stream
```

---

## 3. Type-Erased Access (via Registry::MetadataHolder)

For scenarios where you don't know the type at compile time:

```cpp
auto* holder = registry.get_by_name("MyClass");

// Metadata queries
holder->get_name()                           // Class name           → std::string
holder->get_cpp_type_name()                  // Demangled C++ type   → std::string
holder->get_base_class()                     // Base class name      → std::string
holder->get_inheritance()                    // Inheritance info     → InheritanceInfo&
holder->get_constructors()                   // Constructor metadata → std::vector<ConstructorMeta>

// Field access (via void*)
holder->has_field("name")                    // → bool
holder->get_fields()                         // → std::vector<std::string>
holder->get_field_type("name")               // → std::type_index
holder->get_field_void_ptr(obj_ptr, "name")  // → Any
holder->set_field_void_ptr(obj_ptr, "name", value)

// Property access (via void*)
holder->has_property("name")                 // → bool
holder->get_properties()                     // → std::vector<std::string>
holder->get_property_type("name")            // → std::type_index
holder->get_property_info("name")            // → PropertyMeta
holder->get_property_void_ptr(obj_ptr, "name")       // → Any
holder->set_property_void_ptr(obj_ptr, "name", value)

// Method access (via void*)
holder->has_method("name")                   // → bool
holder->get_methods()                        // → std::vector<std::string>
holder->get_method_info("name")              // → MethodMeta
holder->get_method_arity("name")             // → size_t
holder->get_method_arg_types("name")         // → std::vector<std::type_index>
holder->get_method_return_type("name")       // → std::type_index
holder->invoke_method_void_ptr(obj_ptr, "name", args)       // → Any
holder->invoke_const_method_void_ptr(obj_ptr, "name", args) // → Any
```

---

## 4. Supported Member Types

| Category | Types |
|----------|-------|
| **Primitives** | `bool`, `int`, `long`, `size_t`, `float`, `double`, `uint32_t`, `std::string` |
| **STL Containers** | `std::vector<T>`, `std::array<T,N>`, `std::map<K,V>`, `std::set<T>`, `std::optional<T>` |
| **Smart Pointers** | `std::shared_ptr<T>`, `std::unique_ptr<T>`, `std::weak_ptr<T>`, raw pointers (`T*`) |
| **References** | `T&`, `const T&` |
| **Functors** | `std::function<Ret(Args...)>` |
| **Nested Objects** | Any Rosetta-registered class |

---

## 5. Complete Registration Example

```cpp
#include <rosetta/rosetta.h>

namespace mylib {
    class Vector3 {
    public:
        double x, y, z;
        Vector3() : x(0), y(0), z(0) {}
        Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
        double length() const { return std::sqrt(x*x + y*y + z*z); }
        void normalize() { double l = length(); x/=l; y/=l; z/=l; }
        double operator[](int i) const { return (&x)[i]; }
    };

    class Solver {
    public:
        virtual void solve() = 0;
        virtual ~Solver() = default;
    };

    class IterativeSolver : public Solver {
    protected:
        double tolerance_ = 1e-6;
        int maxIterations_ = 100;
    public:
        double getTolerance() const { return tolerance_; }
        void setTolerance(double t) { tolerance_ = t; }
        int getMaxIterations() const { return maxIterations_; }
        void setMaxIterations(int n) { maxIterations_ = n; }
    };

    class MySolver : public IterativeSolver {
    public:
        void solve() override { /* implementation */ }
        void solve(double hint) { /* overloaded */ }
    };
}

void register_mylib() {
    using namespace mylib;

    rosetta_registry().register_class<Vector3>("Vector3")
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3::x)
        .field("y", &Vector3::y)
        .field("z", &Vector3::z)
        .method("length", &Vector3::length)
        .method("normalize", &Vector3::normalize)
        // Lambda methods for Python-style access
        .lambda_method<double>("__getitem__", [](const Vector3& v, int i) { return v[i]; })
        .lambda_method<std::string>("__repr__", [](const Vector3& v) {
            return "Vector3(" + std::to_string(v.x) + ", " + 
                               std::to_string(v.y) + ", " + 
                               std::to_string(v.z) + ")";
        });

    rosetta_registry().register_class<Solver>("Solver")
        .pure_virtual_method<void>("solve");

    rosetta_registry().register_class<IterativeSolver>("IterativeSolver")
        .inherits_from<Solver>("Solver")
        .property("tolerance", &IterativeSolver::getTolerance, &IterativeSolver::setTolerance)
        .property("maxIterations", &IterativeSolver::getMaxIterations, &IterativeSolver::setMaxIterations)
        .override_method("solve", &IterativeSolver::solve);

    rosetta_registry().register_class<MySolver>("MySolver")
        .inherits_from<IterativeSolver>("IterativeSolver")
        .constructor<>()
        .overloaded_method("solve", ROSETTA_OVERLOAD(MySolver, void, solve))
        .overloaded_method("solve", ROSETTA_OVERLOAD(MySolver, void, solve, double));
}
```