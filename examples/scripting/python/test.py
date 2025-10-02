import pyrosetta as rosetta

# Create objects using constructors
person = rosetta.Person("Alice", 30, 1.65)
vehicle = rosetta.Vehicle("Honda", "Civic", 2022)

# Use auto-generated properties
print(f"Person name: {person.name}")
print(f"Person age: {person.age}")
print(f"Vehicle brand: {vehicle.brand}")

# Modify properties
person.name = "Bob"
person.age = 25
vehicle.mileage = 15000.5

# Call methods
person.introduce()
person.celebrate_birthday()  # Note: pybind11 converts camelCase to snake_case

vehicle.start()
vehicle.drive(100.5)
print(vehicle.get_info())

# Use introspection utilities
print("Class name:", person.get_class_name())
print("Members:", person.get_member_names())
print("Methods:", person.get_method_names())

# Dynamic access
print("Age via introspection:", person.get_member_value("age"))
person.set_member_value("height", 1.80)
person.call_method("introduce", [])

# JSON export
print("Person as JSON:", person.to_json())

# Module utilities
print("Available classes:", rosetta.get_all_classes())
default_person = rosetta.create_person()
default_vehicle = rosetta.create_vehicle()