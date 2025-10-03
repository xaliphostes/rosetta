# Automatic Lua Bindings for C++ Introspection

This directory contains an automatic binding generator that creates Lua bindings from C++ classes using the introspection system and Sol3. Just like the Python and JavaScript versions, it requires minimal code to expose entire C++ classes to Lua.

## Features

- ✅ **Natural Lua property access**: `person.name = "Alice"`
- ✅ **Method calls**: `person:introduce()`
- ✅ **Full introspection from Lua**
- ✅ **Automatic type conversion**
- ✅ **Multiple constructors support**
- ✅ **Getter/setter methods**
- ✅ **Error handling with Lua exceptions**

## Prerequisites

- Lua 5.1+ (or LuaJIT)
- Sol3 (automatically fetched via CMake)
- C++20 compatible compiler
- CMake 3.15 or later

## Building

### Linux/macOS

```bash
# Install Lua development files
# Ubuntu/Debian:
sudo apt-get install liblua5.4-dev

# macOS:
brew install lua

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run
./luarosetta
```

### Windows

```bash
# Download and install Lua from lua.org
# Or use vcpkg:
vcpkg install lua sol2

# Build
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
cmake --build .

# Run
luarosetta.exe
```

## Quick Start

### 1. Define Your C++ Classes

```cpp
class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)
public:
    Person();
    Person(const std::string& n, int a, double h);
    
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

### 2. Create Lua Bindings (Only 1 line!)

```cpp
#include <sol/sol.hpp>
#include <rosetta/generators/lua.h>

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    
    // Bind classes automatically
    rosetta::LuaGenerator(lua).bind_classes<Person, Vehicle>();
    
    // Run Lua script
    lua.script_file("test.lua");
    return 0;
}
```

### 3. Use from Lua

```lua
-- Create objects using constructors
person = Person.new("Alice", 30, 1.65)
vehicle = Vehicle.new("Honda", "Civic", 2022)

-- Natural property access
person.name = "Bob"
person.age = 25
print(person.name)  -- "Bob"
print(person.age)   -- 25

-- Method calls (use colon syntax for methods)
person:introduce()
person:celebrateBirthday()

-- Getter/setter methods
print(person:getName())
person:setName("Charlie")

-- Introspection
print(person:getClassName())        -- "Person"
members = person:getMemberNames()   -- {"name", "age", "height"}
methods = person:getMethodNames()   -- {"introduce", "getName", ...}

-- Dynamic access
person:setMemberValue("age", 35)
print(person:getMemberValue("age"))  -- 35

-- Call methods dynamically
person:callMethod("introduce", {})
desc = person:callMethod("getDescription", {})

-- JSON export
print(person:toJSON())
```

## Performance

The binding generator optimizes for:
- **Fast Property Access**: Direct member access via Sol3
- **Efficient Method Calls**: Minimal overhead for method invocation
- **Type Conversion**: Optimized conversion between Lua and C++ types
- **Memory Management**: Automatic cleanup using shared_ptr

### Benchmarks

```lua
local startTime = os.clock()
for i = 1, 100000 do
    person.age = i
    local _ = person.age
end
local endTime = os.clock()
print(string.format("100k property operations: %.2fms", 
                    (endTime - startTime) * 1000))
```

## Comparison with Manual Bindings

### Manual Sol3 Code (100+ lines)

```cpp
sol::state lua;

lua.new_usertype<Person>("Person",
    sol::constructors<Person(), Person(const std::string&, int, double)>(),
    "name", sol::property(&Person::getName, &Person::setName),
    "age", sol::property(&Person::getAge, &Person::setAge),
    "height", sol::property(&Person::getHeight, &Person::setHeight),
    "introduce", &Person::introduce,
    "celebrateBirthday", &Person::celebrateBirthday,
    "getDescription", &Person::getDescription
    // ... repeat for every member and method
);

// Repeat for every class...
```

### Automatic Bindings (1 line)

```cpp
rosetta::LuaGenerator(lua).bind_classes<Person, Vehicle>();
```

## Advanced Usage

### Embedding Lua in C++

```cpp
#include <sol/sol.hpp>
#include <rosetta/generators/lua.h>

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
    
    // Bind your classes
    rosetta::LuaGenerator generator(lua);
    generator.bind_classes<Person, Vehicle, GameObject>()
             .add_utilities();
    
    // Execute Lua code from C++
    lua.script(R"(
        person = Person.new("Alice", 30, 1.65)
        person:introduce()
    )");
    
    // Get Lua variables in C++
    Person& person = lua["person"];
    std::cout << "From C++: " << person.getName() << std::endl;
    
    // Call Lua functions from C++
    lua.script(R"(
        function processData(p)
            p.age = p.age + 1
            return p:getDescription()
        end
    )");
    
    sol::function process = lua["processData"];
    std::string result = process(person);
    std::cout << result << std::endl;
    
    return 0;
}
```

### Dynamic Class Loading

```lua
-- Load classes dynamically based on configuration
local classConfig = {
    {type = "Person", name = "Alice", age = 30, height = 1.65},
    {type = "Vehicle", brand = "Honda", model = "Civic", year = 2022}
}

local objects = {}
for i, config in ipairs(classConfig) do
    if config.type == "Person" then
        objects[i] = Person.new(config.name, config.age, config.height)
    elseif config.type == "Vehicle" then
        objects[i] = Vehicle.new(config.brand, config.model, config.year)
    end
end

-- Process all objects
for i, obj in ipairs(objects) do
    print(obj:getClassName())
    print(obj:toJSON())
end
```

### Batch Operations

```lua
-- Update multiple objects
function batchUpdate(objects, property, value)
    for i, obj in ipairs(objects) do
        if obj:hasMember(property) then
            obj:setMemberValue(property, value)
        end
    end
end

local people = {
    Person.new("Alice", 30, 1.65),
    Person.new("Bob", 25, 1.80),
    Person.new("Charlie", 35, 1.75)
}

batchUpdate(people, "age", 40)
```

### Custom Type Converters

For custom C++ types, you can extend the type conversion:

```cpp
// In your binding code
generator.register_type_converter(
    "Vector3D",
    // C++ to Lua
    [](sol::state_view lua, const std::any& value) -> sol::object {
        auto vec = std::any_cast<Vector3D>(value);
        sol::table t = lua.create_table();
        t["x"] = vec.x;
        t["y"] = vec.y;
        t["z"] = vec.z;
        return t;
    },
    // Lua to C++
    [](sol::object lua_val) -> std::any {
        sol::table t = lua_val.as<sol::table>();
        return Vector3D(t["x"], t["y"], t["z"]);
    }
);
```

## Integration with Game Engines

Lua is commonly used for game scripting. Here's how to integrate:

```cpp
// game_engine.cpp
class GameEngine {
    sol::state lua;
    rosetta::LuaGenerator* generator;
    
public:
    void initialize() {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
        
        // Bind all game classes
        generator = new rosetta::LuaGenerator(lua);
        generator->bind_classes<GameObject, Player, Enemy, Weapon>();
        
        // Load game scripts
        lua.script_file("scripts/game_logic.lua");
    }
    
    void update(float deltaTime) {
        // Call Lua update function
        lua["update"](deltaTime);
    }
    
    void onEvent(const std::string& eventName) {
        lua["onEvent"](eventName);
    }
};
```

```lua
-- scripts/game_logic.lua
player = Player.new("Hero", 100, 10)
enemies = {}

function update(deltaTime)
    -- Update game logic
    player:update(deltaTime)
    
    for i, enemy in ipairs(enemies) do
        enemy:update(deltaTime)
        
        -- Check collision
        if checkCollision(player, enemy) then
            player:takeDamage(enemy:getDamage())
        end
    end
end

function onEvent(eventName)
    if eventName == "spawn_enemy" then
        table.insert(enemies, Enemy.new("Goblin", 50, 5))
    end
end
```

## Troubleshooting

### Build Issues

```bash
# Check Lua installation
lua -v

# On Ubuntu/Debian, if Lua is not found:
sudo apt-get install liblua5.4-dev pkg-config

# On macOS:
brew install lua

# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

### Runtime Errors

```lua
-- Check if class is bound
status, err = pcall(function()
    local p = Person.new()
end)

if not status then
    print("Person class not bound: " .. err)
end

-- Debug introspection
print("Available classes:")
for i, name in ipairs(getAllClasses()) do
    print("  " .. name)
end
```

### Memory Issues

Sol3 handles memory automatically, but be aware:

```lua
-- Lua holds references
local person = Person.new("Alice", 30, 1.65)
-- person will be garbage collected when no longer referenced

-- Weak references if needed
local weakTable = setmetatable({}, {__mode = "v"})
weakTable[1] = Person.new("Bob", 25, 1.75)
```

## Examples

See `test.lua` for comprehensive usage examples including:
- Basic property and method access
- Constructor usage
- Error handling
- Dynamic introspection
- Performance testing
- Batch operations
- JSON serialization

## License

LGPL License - see LICENSE file for details

## Contributing

Contributions are welcome! The Lua bindings follow the same pattern as Python and JavaScript bindings, making it easy to maintain consistency across all three languages.

## Support

For issues, questions, or contributions:
- GitHub Issues: [link to repo]
- Documentation: [link to docs]
- Examples: See `examples/scripting/lua/` directory