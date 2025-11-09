#!/usr/bin/env python3
"""
Test script demonstrating std::map, std::set, and std::array support
"""

import rosetta_example as ce

print("=" * 60)
print("Testing DataContainer with various container types")
print("=" * 60)

# Create a DataContainer instance
container = ce.DataContainer()

# Test vector (list in Python)
print("\n1. Testing std::vector -> Python list")
container.values = [1.5, 2.5, 3.5, 4.5]
print(f"Values: {container.values}")
container.addValue(5.5)
print(f"After addValue(5.5): {container.values}")
print(f"Value count: {container.getValueCount()}")

# Test map (dict in Python)
print("\n2. Testing std::map -> Python dict")
container.name_to_id = {"Alice": 1, "Bob": 2, "Charlie": 3}
print(f"Name to ID mapping: {container.name_to_id}")
container.addMapping("David", 4)
print(f"After addMapping('David', 4): {container.name_to_id}")
print(f"Alice's ID: {container.getId('Alice')}")
print(f"Eve's ID: {container.getId('Eve')}")  # Should return -1
print(f"Mapping count: {container.getMappingCount()}")

# Test set (set in Python)
print("\n3. Testing std::set -> Python set")
container.tags = {"important", "urgent", "review"}
print(f"Tags: {container.tags}")
container.addTag("completed")
print(f"After addTag('completed'): {container.tags}")
print(f"Has 'urgent' tag: {container.hasTag('urgent')}")
print(f"Has 'archived' tag: {container.hasTag('archived')}")
print(f"Tag count: {container.getTagCount()}")

# Test array (list/tuple in Python)
print("\n4. Testing std::array -> Python list")
container.position = [1.0, 2.0, 3.0]
print(f"Position: {container.position}")
print(f"Position type: {type(container.position)}")

print("\n" + "=" * 60)
print("Testing MathUtils with container operations")
print("=" * 60)

math = ce.MathUtils()

# Test array return
print("\n5. Testing array return values")
fib = math.getFibonacci4()
print(f"Fibonacci sequence (first 4): {fib}")
print(f"Type: {type(fib)}")

# Test set return
print("\n6. Testing set return values")
primes = math.getPrimesUpTo20()
print(f"Prime numbers up to 20: {sorted(primes)}")
print(f"Type: {type(primes)}")

# Test map return
print("\n7. Testing map operations")
numbers = [1, 2, 2, 3, 3, 3, 4, 4, 4, 4]
freq = math.getFrequencies(numbers)
print(f"Frequencies of {numbers}:")
for num, count in sorted(freq.items()):
    print(f"  {num}: {count} times")

# Test map transformation
print("\n8. Testing map transformation")
prices = {"apple": 1.5, "banana": 0.8, "orange": 2.0}
print(f"Original prices: {prices}")
scaled = math.scaleMap(prices, 1.1)  # 10% increase
print(f"After 10% increase: {scaled}")

# Test set operations
print("\n9. Testing set operations")
set_a = {1, 2, 3, 4, 5}
set_b = {4, 5, 6, 7, 8}
print(f"Set A: {set_a}")
print(f"Set B: {set_b}")
intersection = math.setIntersection(set_a, set_b)
print(f"Intersection: {intersection}")
union = math.setUnion(set_a, set_b)
print(f"Union: {union}")

# Test array operations
print("\n10. Testing array operations")
v1 = [3.0, 4.0, 0.0]
print(f"Vector v1: {v1}")
normalized = math.normalizeVector(v1)
print(f"Normalized: {normalized}")
print(f"Length check: {sum(x*x for x in normalized):.6f} (should be ~1.0)")

v2 = [1.0, 0.0, 0.0]
dot = math.dotProduct(v1, v2)
print(f"Dot product of {v1} and {v2}: {dot}")

# Test edge cases
print("\n11. Testing edge cases")
empty_map = {}
container.name_to_id = empty_map
print(f"Empty map: {container.name_to_id}")
print(f"Mapping count: {container.getMappingCount()}")

empty_set = set()
container.tags = empty_set
print(f"Empty set: {container.tags}")
print(f"Tag count: {container.getTagCount()}")

zero_vector = [0.0, 0.0, 0.0]
normalized_zero = math.normalizeVector(zero_vector)
print(f"Normalized zero vector: {normalized_zero}")

print("\n" + "=" * 60)
print("All tests completed successfully!")
print("=" * 60)