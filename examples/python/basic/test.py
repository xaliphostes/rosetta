import basic

print("Testing Rosetta Python bindings")

# Create a Vector3D and compute length
v1 = basic.Vector3D()
print("Initial vector")

v1.x = 3.0
v1.y = 4.0
v1.z = 0.0
print("Length: ",v1.length())  # Output: Length: 5.0

v1.normalize()
print("After normalize: ", v1.to_string())

# Create another vector and add
v2 = basic.Vector3D()
print("1")
v2.x = 1.0
v2.y = 1.0
v2.z = 1.0
print("2")

v3 = v1.add(v2)
print("3")
print("Sum: ", v3.to_string())

# Work with shapes
circle = basic.Circle()
circle.radius = 5.0
print("Circle area: ", circle.area())
print("Circle name: ", circle.name())

rect = basic.Rectangle()
rect.width = 3.0
rect.height = 4.0
print("Rectangle area: ", rect.area())

# Work with Person
person = basic.Person()
person.name = "Alice"
person.age = 30
person.add_hobby("reading")
person.add_hobby("hiking")

print(person.introduce())
print("Hobbies: ", person.get_hobbies())

# List all registered classes
print("Available classes: ", basic.list_classes())

# Get version
print("Rosetta version: " ,basic.get_version())