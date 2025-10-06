# Enum Support in Rosetta

## Quick Start

### Method 1: BEGIN/END Syntax (Recommended - Always Works)

```cpp
enum class Status { Active, Inactive, Pending };

BEGIN_ENUM_REGISTRATION(Status)
    ENUM_VALUE(Active)
    ENUM_VALUE(Inactive)
    ENUM_VALUE(Pending)
END_ENUM_REGISTRATION()
```

### Method 2: One-Liner (Convenient for 3 values)

```cpp
enum class Status { Active, Inactive, Pending };
REGISTER_ENUM(Status, Active, Inactive, Pending);  // Exactly 3 values

// For 4 values:
REGISTER_ENUM_4(Priority, Low, Medium, High, Critical);

// For 5 values:
REGISTER_ENUM_5(Size, XSmall, Small, Medium, Large, XLarge);
```

### Method 3: Plain Enums (not enum class)

```cpp
enum Color { Red, Green, Blue };

BEGIN_PLAIN_ENUM_REGISTRATION(Color)
    PLAIN_ENUM_VALUE(Red)
    PLAIN_ENUM_VALUE(Green)
    PLAIN_ENUM_VALUE(Blue)
END_PLAIN_ENUM_REGISTRATION()
```

## Complete Example

```cpp
#include <rosetta/rosetta.h>
#include <rosetta/generators/js.h>

// Define enums
enum class Status { Active, Inactive, Pending };
enum class Priority { Low, Medium, High, Critical };

// Register - choose one method:

// Option A: Simple macro
REGISTER_ENUM(Status, Active, Inactive, Pending);

// Option B: BEGIN/END syntax (more reliable)
BEGIN_ENUM_REGISTRATION(Priority)
    ENUM_VALUE(Low)
    ENUM_VALUE(Medium)
    ENUM_VALUE(High)
    ENUM_VALUE(Critical)
END_ENUM_REGISTRATION()

// Use in a class
class Task : public rosetta::Introspectable {
    INTROSPECTABLE(Task)
public:
    Task() : status_(Status::Pending), priority_(Priority::Medium) {}
    
    Status getStatus() const { return status_; }
    void setStatus(Status s) { status_ = s; }
    
    Priority getPriority() const { return priority_; }
    void setPriority(Priority p) { priority_ = p; }

private:
    Status status_;
    Priority priority_;
};

void Task::registerIntrospection(rosetta::TypeRegistrar<Task> reg) {
    reg.constructor<>()
       .member("status", &Task::status_)
       .member("priority", &Task::priority_)
       .method("getStatus", &Task::getStatus)
       .method("setStatus", &Task::setStatus)
       .method("getPriority", &Task::getPriority)
       .method("setPriority", &Task::setPriority);
}

// JavaScript binding
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);
    
    // Register enums
    rosetta::registerEnumType<Status>(generator);
    rosetta::registerEnumType<Priority>(generator);
    
    // Bind class
    generator.bind_class<Task>();
    
    return exports;
}

NODE_API_MODULE(task_module, Init)
```

## JavaScript Usage

```javascript
const addon = require('./build/Release/task_module');

// Enum objects are available
console.log(addon.Status);
// { Active: 0, Inactive: 1, Pending: 2 }

console.log(addon.Priority);
// { Low: 0, Medium: 1, High: 2, Critical: 3 }

// Create task
const task = new addon.Task();

// Use enums
task.setStatus(addon.Status.Active);
console.log(task.getStatus()); // 0

// Direct property access
task.status = addon.Status.Inactive;
task.priority = addon.Priority.High;

// Enums are immutable
addon.Status.NewValue = 99; // Error!
```

## Python Usage

```python
import task_module

# Access enums
print(task_module.Status.Active)  # Status.Active
print(task_module.Status.Active.value)  # 0

# Create task
task = task_module.Task()

# Use enums
task.set_status(task_module.Status.Active)
print(task.get_status())  # Status.Active

# Compare enums
if task.get_status() == task_module.Status.Active:
    print('Active!')

# Iterate enum values
for status in task_module.Status:
    print(f'{status.name}: {status.value}')
```

## Lua Usage

```lua
local addon = require('task_module')

-- Access enums
print(addon.Status.Active)  -- 0
print(addon.Priority.High)  -- 2

-- Create task
local task = addon.Task.new()

-- Use enums
task:setStatus(addon.Status.Active)
print(task:getStatus())  -- 0

-- Enums are read-only
-- addon.Status.NewValue = 99  -- Error!
```

## Troubleshooting

### Compiler Error: "identifier not found"

**Problem:** `REGISTER_ENUM(Status, Active, ...)` fails with "Active: identifier not found"

**Solution:** Use the BEGIN/END syntax instead:

```cpp
// Instead of:
// REGISTER_ENUM(Status, Active, Inactive, Pending);

// Use:
BEGIN_ENUM_REGISTRATION(Status)
    ENUM_VALUE(Active)
    ENUM_VALUE(Inactive)
    ENUM_VALUE(Pending)
END_ENUM_REGISTRATION()
```

### Plain Enum (not enum class)

**Problem:** `enum Color { Red, Green, Blue };` doesn't work with `REGISTER_ENUM`

**Solution:** Use `REGISTER_PLAIN_ENUM`:

```cpp
enum Color { Red, Green, Blue };
REGISTER_PLAIN_ENUM(Color, Red, Green, Blue);
```

### Custom Values

All methods support custom enum values:

```cpp
enum class HttpStatus {
    OK = 200,
    NotFound = 404,
    ServerError = 500
};

BEGIN_ENUM_REGISTRATION(HttpStatus)
    ENUM_VALUE(OK)           // 200
    ENUM_VALUE(NotFound)     // 404
    ENUM_VALUE(ServerError)  // 500
END_ENUM_REGISTRATION()
```

## Advanced Features

### Get Enum Name from Value

```cpp
auto& registry = rosetta::EnumRegistry::instance();
const auto* info = registry.getEnumInfo<Status>();

std::string name = info->getName(0);  // "Active"
int64_t value = info->getValue("Active");  // 0
```

### Check if Value Exists

```cpp
if (info->hasValue("Active")) {
    // ...
}

if (info->hasValue(0)) {
    // ...
}
```

### List All Enum Values

```cpp
for (const auto& value_info : info->values) {
    std::cout << value_info.name << " = " 
              << value_info.value << "\n";
}
```

### Get All Registered Enums

```cpp
auto& registry = rosetta::EnumRegistry::instance();
for (const auto& enum_name : registry.getAllEnumNames()) {
    std::cout << "Enum: " << enum_name << "\n";
}
```

## Best Practices

1. **Register enums before binding classes** that use them
2. **Use BEGIN/END syntax** for maximum compatibility
3. **Group enum registrations** at the top of your binding file
4. **Use meaningful enum names** that describe the domain
5. **Document enum values** in comments

```cpp
// Good practice
enum class TaskStatus {
    Pending = 0,    // Initial state
    Running = 1,    // Currently executing
    Completed = 2,  // Successfully finished
    Failed = 3      // Encountered error
};

BEGIN_ENUM_REGISTRATION(TaskStatus)
    ENUM_VALUE(Pending)
    ENUM_VALUE(Running)
    ENUM_VALUE(Completed)
    ENUM_VALUE(Failed)
END_ENUM_REGISTRATION()
```