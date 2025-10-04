# TODO list

## Enhanced Type System Support

- Smart Pointers
    ```cpp
    // Support for shared_ptr, unique_ptr, weak_ptr
    .method("getSharedResource", &Class::getSharedResource)  // returns shared_ptr<T>
    .method("createUnique", &Class::createUnique)  // returns unique_ptr<T>
    ```

- Other STL Containers such as map, set...
    ```cpp
    // std::map, std::unordered_map support
    registerMapType<std::string, int>(generator);

    // std::set, std::unordered_set
    registerSetType<std::string>(generator);

    // std::optional, std::variant
    registerOptionalType<Person>(generator);
    ```

- Enum Support
    ```cpp
    enum class Status { Active, Inactive, Pending };
    REGISTER_ENUM(Status, Active, Inactive, Pending);

    // Auto-generates bidirectional conversion
    ```
  
## Property System Enhancements

- Computed Properties
  ```cpp
    reg.computed_property("fullName", 
        [](const Person& p) { return p.firstName + " " + p.lastName; },
        [](Person& p, const std::string& name) { /* parse and set */ }
    );
  ```

- Property Attributes/Metadata
  ```cpp
    reg.member("age", &Person::age)
        .readonly()     // No setter in bindings
        .range(0, 150)  // Validation
        .description("Person's age in years");
  ```

- Property Change Notifications
  ```cpp
    reg.constructor<const std::string&, int>()
      .member_observable("name", &Person::name)  // Observable!
      .member_observable("age", &Person::age)    // Observable!
      .method("getName", &Person::getName)
      .method("setName", &Person::setName)
      .method("getAge", &Person::getAge)
      .method("setAge", &Person::setAge);

    // C++ usage
    Person person("Alice", 30);
    person.propertyChanged.subscribe([](const std::string& prop, 
                                      const std::any& oldVal, 
                                      const std::any& newVal) {
        std::cout << "Property '" << prop << "' changed from " 
                  << std::any_cast<int>(oldVal) << " to " 
                  << std::any_cast<int>(newVal) << std::endl;
    });

    person.setAge(31);  // Triggers notification
  ```

## Advanced Method Features

- Method Overloading
  ```cpp
    cppreg.method("setValue", static_cast<void(Class::*)(int)>(&Class::setValue))
        .method("setValue", static_cast<void(Class::*)(double)>(&Class::setValue))
        .method("setValue", static_cast<void(Class::*)(const std::string&)>(&Class::setValue));
  ```

- Default Arguments
  ```cpp
    reg.method("process", &Class::process)
        .default_args(100, "default");  // Last 2 params optional
  ```

- Operator Overloading
  ```cpp
    reg.operator_add(&Vector3::operator+)
        .operator_multiply(&Vector3::operator*)
        .operator_index(&Container::operator[]);
  ```

## Inheritance & Polymorphism

## Async/Callback Support

## Memory Management Enhancements

## Serialization/Deserialization

## Debugging & Development Tools

- Runtime Type Information
    ```cpp
        auto typeInfo = rosetta::getTypeInfo("Person");
        // Get all registered types dynamically
    ```

- Hot Reload Support
    ```cpp
        // Detect C++ class changes and reload bindings
        generator.enable_hot_reload();
    ```

- Profiling/Tracing
    ```cpp
    reg.method("expensiveOp", &Class::expensiveOp)
        .trace();  // Auto-log execution time
    ```

## Cross-Language Features

- TypeScript Definition Generation
    ```cpp
    // Auto-generate .d.ts files from introspection
    rosetta::generateTypeScriptDefinitions<Person, Vehicle>("bindings.d.ts");
    ```

- Documentation Generation
    ```cpp
    reg.method("calculate", &Class::calculate)
        .doc("Calculates the result based on input parameters")
        .param_doc(0, "x", "The first operand")
        .param_doc(1, "y", "The second operand")
        .returns_doc("The calculated result");
    ```

## Module System

- Namespace Support
    ```cpp
    rosetta::Namespace physics(generator);
    physics.bind_class<Vector3>();
    physics.bind_class<Quaternion>();

    // In JS: const v = new physics.Vector3()
    ```

## Template/Generic Support

- Template Class Registration
    ```cpp
    template<typename T>
    class Container : public rosetta::Introspectable {
        INTROSPECTABLE_TEMPLATE(Container, T)
    };

    // Register specific instantiations
    generator.bind_class<Container<int>>("ContainerInt");
    generator.bind_class<Container<std::string>>("ContainerString");
    ```
