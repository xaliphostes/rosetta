# Quick Start Guide - Unified Python Binding

## 🚀 Quick Start

### Step 1: Register Your Classes

```cpp
#include <rosetta/rosetta.h>
#include <rosetta/generators/python_binder_unified.h>

// Register with Rosetta AND Python binding in one step
ROSETTA_REGISTER_CLASS_PY(Person)
    .constructor<>()
    .constructor<std::string, int>()
    .field("name", &Person::name)
    .field("age", &Person::age)
    .method("greet", &Person::greet)
    .static_method("create", &Person::create);

ROSETTA_REGISTER_CLASS_PY(Vehicle)
    .constructor<std::string>()
    .field("model", &Vehicle::model)
    .method("drive", &Vehicle::drive);
```

### Step 2: Create Python Module - Choice of 3 Styles

#### Style A: Automatic Binding (Simplest)
```cpp
ROSETTA_PY_MODULE(mymodule, "Auto-generated Python bindings") {
    BIND_ALL_CLASSES();  // Binds Person, Vehicle, everything!
}
ROSETTA_PY_MODULE_END
```

#### Style B: Manual Binding (Full Control)
```cpp
ROSETTA_PY_MODULE(mymodule, "Manual Python bindings") {
    BIND_CLASS(Person);
    BIND_CLASS(Vehicle);
    BIND_FUNCTION(compute, "Compute something");
    BIND_CONSTANT("PI", 3.14159);
}
ROSETTA_PY_MODULE_END
```

#### Style C: Hybrid (Recommended!)
```cpp
ROSETTA_PY_MODULE(mymodule, "Smart Python bindings") {
    // Auto-bind most classes, but filter out internal ones
    binder.bind_all_registered([](const std::string& name) {
        return !name.starts_with("Internal");
    });
    
    // Manually bind special cases with custom names
    binder.bind_class<SpecialClass>("CustomPythonName");
    
    // Add utilities
    BIND_UTILITIES();  // Adds list_classes(), get_class_info(), version()
}
ROSETTA_PY_MODULE_END
```

### Step 3: Build and Use

```bash
# Build your Python module
python setup.py build_ext --inplace

# Use in Python
python3
>>> import mymodule
>>> p = mymodule.Person("Alice", 30)
>>> p.greet()
'Hello, I am Alice'
>>> p.age = 31
>>> print(p.age)
31
```

## 📚 Common Patterns

### Pattern 1: Simple Library
```cpp
// Register all your classes
ROSETTA_REGISTER_CLASS_PY(Class1) /* ... */;
ROSETTA_REGISTER_CLASS_PY(Class2) /* ... */;
ROSETTA_REGISTER_CLASS_PY(Class3) /* ... */;

// One-line Python module
ROSETTA_PY_MODULE(mylib, "My library") { BIND_ALL_CLASSES(); }
ROSETTA_PY_MODULE_END
```

### Pattern 2: Public API Only
```cpp
// Register everything (including internal classes)
ROSETTA_REGISTER_CLASS_PY(PublicClass) /* ... */;
ROSETTA_REGISTER_CLASS_PY(InternalHelper) /* ... */;
ROSETTA_REGISTER_CLASS_PY(InternalUtil) /* ... */;

// But only expose public API to Python
ROSETTA_PY_MODULE(mylib, "Public API") {
    binder.bind_all_registered([](const std::string& name) {
        return name.find("Internal") == std::string::npos;
    });
}
ROSETTA_PY_MODULE_END
```

### Pattern 3: Custom Types
```cpp
struct MyCustomType {
    int value;
};

// Register custom type converter ONCE
void register_custom_types() {
    rosetta::python::TypeConverter::register_type<MyCustomType>();
}

// Now it works everywhere automatically!
ROSETTA_REGISTER_CLASS_PY(MyClass)
    .field("custom", &MyClass::custom);  // MyCustomType field - works!

ROSETTA_PY_MODULE(mymodule, "With custom types") {
    register_custom_types();  // Call once at module init
    BIND_ALL_CLASSES();
}
ROSETTA_PY_MODULE_END
```

## 🔧 API Reference

### PythonBinder Methods

```cpp
PythonBinder binder(module);

// Manual binding
binder.bind_class<T>("name");           // Bind specific class
binder.bind_function("name", ptr, doc); // Bind free function
binder.bind_constant("name", value);    // Bind constant

// Automatic binding
binder.bind_all_registered();           // Bind all registered classes
binder.bind_all_registered(filter);     // Bind with filter function

// Utilities
binder.add_utilities();                 // Add introspection functions
binder.set_doc("documentation");        // Set module doc
```

### Macros (Convenience)

```cpp
ROSETTA_PY_MODULE(name, doc)           // Begin module definition
ROSETTA_PY_MODULE_END                  // End module definition

ROSETTA_REGISTER_CLASS_PY(Class)       // Register class for binding

BIND_CLASS(Class)                      // Bind class (inside module)
BIND_ALL_CLASSES()                     // Bind all registered
BIND_FUNCTION(func, doc)               // Bind function
BIND_CONSTANT(name, value)             // Bind constant
BIND_UTILITIES()                       // Add utility functions
```

## 🎯 Migration from Old System

### From `py_generator.h`

```cpp
// OLD
BEGIN_PY_MODULE(mymodule, "Doc") {
    BIND_PY_CLASS(Person);
}
END_PY_MODULE()

// NEW (nearly identical!)
ROSETTA_PY_MODULE(mymodule, "Doc") {
    BIND_CLASS(Person);
}
ROSETTA_PY_MODULE_END
```

### From `python_binding_generator.h`

```cpp
// OLD
ROSETTA_REGISTER_CLASS_WITH_PYTHON(Person) /* ... */;

ROSETTA_PYBIND11_MODULE(mymodule) {
    // Auto-binds all
}
END_ROSETTA_PYBIND11_MODULE

// NEW
ROSETTA_REGISTER_CLASS_PY(Person) /* ... */;

ROSETTA_PY_MODULE(mymodule, "Doc") {
    BIND_ALL_CLASSES();
}
ROSETTA_PY_MODULE_END
```

## ⚡ Performance Notes

- **Compile-time**: Same as before (template instantiation)
- **Runtime binding**: Identical performance
- **Type conversion**: Zero overhead (inline functions)
- **Memory**: Less code = smaller binaries

## 🐛 Troubleshooting

### Issue: "Class not registered"
```cpp
// Solution: Use ROSETTA_REGISTER_CLASS_PY, not ROSETTA_REGISTER_CLASS
ROSETTA_REGISTER_CLASS_PY(MyClass)  // ✅ Correct
// not:
ROSETTA_REGISTER_CLASS(MyClass)     // ❌ Won't auto-bind
```

### Issue: "Type conversion failed"
```cpp
// Solution: Register custom type
rosetta::python::TypeConverter::register_type<MyType>();
```

### Issue: "No module named..."
```bash
# Solution: Check build output
python setup.py build_ext --inplace
ls *.so  # Should see mymodule.so (or .pyd on Windows)
```

## 📖 Advanced Topics

### Custom Binding for Special Cases

```cpp
ROSETTA_PY_MODULE(mymodule, "Advanced") {
    // Auto-bind most classes
    BIND_ALL_CLASSES();
    
    // But customize one class further
    auto& py_special = binder.module().attr("SpecialClass");
    py_special.attr("CONSTANT") = 42;
    
    // Or add custom method
    binder.module().def("create_special", []() {
        return SpecialClass::createAdvanced();
    });
}
ROSETTA_PY_MODULE_END
```

### Multiple Modules

```cpp
// module_core.cpp
ROSETTA_PY_MODULE(mylib_core, "Core classes") {
    binder.bind_class<BaseClass>();
    binder.bind_class<CoreClass>();
}
ROSETTA_PY_MODULE_END

// module_extras.cpp  
ROSETTA_PY_MODULE(mylib_extras, "Extra classes") {
    binder.bind_class<ExtraClass>();
}
ROSETTA_PY_MODULE_END

// Python usage:
// import mylib_core
// import mylib_extras
```

## ✅ Checklist

Before deploying:
- [ ] All classes use `ROSETTA_REGISTER_CLASS_PY`
- [ ] Custom types registered with `TypeConverter::register_type<T>()`
- [ ] Module created with `ROSETTA_PY_MODULE` / `ROSETTA_PY_MODULE_END`
- [ ] Tested with `BIND_UTILITIES()` to verify all classes bound
- [ ] Python import works: `import mymodule`
- [ ] Can create instances: `obj = mymodule.MyClass()`

## 🎓 Learn More

- **REFACTORING_GUIDE.md** - Why we refactored, what changed
- **ARCHITECTURE.md** - Design patterns, data flow diagrams
- **Rosetta docs** - Understanding the introspection system

## 💡 Tips

1. **Start simple**: Use `BIND_ALL_CLASSES()` first
2. **Add filters**: Exclude internal classes as needed
3. **Custom when needed**: Fall back to manual binding for special cases
4. **Test incrementally**: Bind one class, test, then bind more
5. **Use utilities**: `BIND_UTILITIES()` helps debug binding issues

---

**You're ready!** The unified system gives you the same power with less code. 🚀