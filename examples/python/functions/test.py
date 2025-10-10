# examples/python/functions/test.py
import rosetta_functions as rf

print("=== Python Function Bindings Test ===\n")

# Test 1: Simple arithmetic function
print("1. Testing add(20, 3):")
result = rf.add(20, 3)
print(f"   Result: {result}")
assert result == 23, "add() failed"
print("   ✓ PASS\n")

# Test 2: String function
print("2. Testing greet('World'):")
result = rf.greet("World")
print(f"   Result: {result}")
assert result == "Hello, World", "greet() failed"
print("   ✓ PASS\n")

# Test 3: Mathematical function
print("3. Testing calculateDistance(0, 0, 3, 4):")
result = rf.calculateDistance(0, 0, 3, 4)
print(f"   Result: {result}")
assert abs(result - 5.0) < 0.001, "calculateDistance() failed"
print("   ✓ PASS\n")

# Test 4: Vector function
print("4. Testing normalize([1, 2, 3, 4]):")
result = rf.normalize([1.0, 2.0, 3.0, 4.0])
print(f"   Result: {result}")
expected_sum = sum(result)
assert abs(expected_sum - 1.0) < 0.001, "normalize() failed"
print(f"   Sum: {expected_sum}")
print("   ✓ PASS\n")

# Test 5: Edge cases
print("5. Testing edge cases:")
try:
    # Empty string
    result = rf.greet("")
    print(f"   greet(''): {result}")
    
    # Negative numbers
    result = rf.add(-5, 10)
    print(f"   add(-5, 10): {result}")
    assert result == 5, "add() with negatives failed"
    
    # Zero distance
    result = rf.calculateDistance(0, 0, 0, 0)
    print(f"   calculateDistance(0, 0, 0, 0): {result}")
    assert result == 0.0, "calculateDistance() with zeros failed"
    
    print("   ✓ PASS\n")
except Exception as e:
    print(f"   ✗ FAIL: {e}\n")

print("=== All Tests Completed ===")