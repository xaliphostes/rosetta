-- examples/scripting/lua/test.lua
-- Comprehensive test suite for automatic Lua bindings

print("=== Automatic Lua Bindings Test ===\n")

-- Test 1: Creating objects
print("1. Creating objects...")
local person = Person.new("Alice", 30, 1.65)
local vehicle = Vehicle.new("Honda", "Civic", 2022)

print("Person created: " .. person:getClassName())
print("Vehicle created: " .. vehicle:getClassName())
print(string.format("Person: %s, age %d, height %.2fm", 
                    person.name, person.age, person.height))
print(string.format("Vehicle: %s %s (%d), %.1f miles", 
                    vehicle.brand, vehicle.model, vehicle.year, vehicle.mileage))
print()

-- Test 2: Direct property access (natural Lua style)
print("2. Direct property access...")
person.name = "Bob"
person.age = 25
person.height = 1.75
person.isActive = true

vehicle.brand = "BMW"
vehicle.model = "Serie 3"
vehicle.year = 2021
vehicle.mileage = 5000.5

print(string.format("Person: %s, age %d, height %.2fm", 
                    person.name, person.age, person.height))
print(string.format("Vehicle: %s %s (%d), %.1f miles", 
                    vehicle.brand, vehicle.model, vehicle.year, vehicle.mileage))
print()

-- Test 3: Method calls
print("3. Method calls...")
person:introduce()
person:celebrateBirthday()
print("Description: " .. person:getDescription())
print()

vehicle:start()
vehicle:drive(100.5)
print("Info: " .. vehicle:getInfo())
vehicle:stop()
print()

-- Test 4: Introspection utilities
print("4. Introspection utilities...")
print("Person class name: " .. person:getClassName())

print("Person members:")
local members = person:getMemberNames()
for i, name in ipairs(members) do
    print("  " .. name)
end

print("Person methods:")
local methods = person:getMethodNames()
for i, name in ipairs(methods) do
    print("  " .. name)
end

print("Has 'name' member: " .. tostring(person:hasMember("name")))
print("Has 'weight' member: " .. tostring(person:hasMember("weight")))
print("Has 'introduce' method: " .. tostring(person:hasMethod("introduce")))
print()

-- Test 5: Dynamic member access
print("5. Dynamic member access...")
print("Age via introspection: " .. tostring(person:getMemberValue("age")))
person:setMemberValue("height", 1.80)
print("Height after dynamic set: " .. tostring(person:getMemberValue("height")))
print()

-- Test 6: Dynamic method calls
print("6. Dynamic method calls...")
person:callMethod("introduce", {})
local description = person:callMethod("getDescription", {})
print("Description from dynamic call: " .. description)
print()

-- Test 7: JSON export
print("7. JSON export...")
print("Person as JSON:")
print(person:toJSON())
print()

-- Test 8: Getter/setter methods
print("8. Getter/setter methods...")
print("Person name via getter: " .. person:getName())
person:setName("Charlie")
print("Person name after setter: " .. person:getName())
print()

-- Test 9: Advanced scenarios
print("9. Advanced scenarios...")

-- Batch property update
local personData = {
    name = "Diana",
    age = 28,
    height = 1.68,
    isActive = false
}

for key, value in pairs(personData) do
    if person:hasMember(key) then
        person:setMemberValue(key, value)
    end
end

print("After batch update: " .. person:getDescription())
print()

-- Test 10: Performance test
print("10. Performance test...")
local startTime = os.clock()
for i = 1, 1000 do
    person:setAge(i % 100)
    local _ = person:getAge()
end
local endTime = os.clock()
print(string.format("1000 property operations took: %.2fms", 
                    (endTime - startTime) * 1000))
print()

-- Test 11: Module utilities
print("11. Module utilities...")
print("Available classes:")
local classes = getAllClasses()
for i, name in ipairs(classes) do
    print("  " .. name)
end
print()

-- Test 12: Error handling
print("12. Error handling...")
local status, err = pcall(function()
    person.nonExistentMember = "value"
end)
if not status then
    print("Expected error caught: Member not found")
end

status, err = pcall(function()
    person:callMethod("nonExistentMethod", {})
end)
if not status then
    print("Expected error caught: Method not found")
end
print()

print("=== All tests completed successfully! ===")