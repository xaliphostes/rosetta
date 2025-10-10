#!/usr/bin/env python3
"""
Helper script to find and test the rosetta_functions module
"""
import os
import sys
import glob

print("=== Finding rosetta_functions module ===\n")

# Search for .pyd files (Windows) or .so files (Linux/Mac)
build_dir = os.path.dirname(os.path.abspath(__file__))
print(f"Searching in: {build_dir}")

# Search patterns
patterns = [
    "**/*.pyd",  # Windows
    "**/*.so",   # Linux/Mac
]

found_modules = []
for pattern in patterns:
    found = glob.glob(os.path.join(build_dir, pattern), recursive=True)
    found_modules.extend(found)

if not found_modules:
    print("\n❌ ERROR: No Python module found!")
    print("\nPlease build the project first:")
    print("  cmake --build . --config Release")
    sys.exit(1)

print(f"\nFound {len(found_modules)} module(s):")
for module in found_modules:
    rel_path = os.path.relpath(module, build_dir)
    size = os.path.getsize(module) / 1024  # KB
    print(f"  - {rel_path} ({size:.1f} KB)")

# Find the most likely candidate (shortest path, or Release over Debug)
def module_priority(path):
    """Lower is better"""
    score = len(path)
    if 'Release' in path:
        score -= 100
    if 'Debug' in path:
        score += 100
    return score

found_modules.sort(key=module_priority)
module_path = found_modules[0]
module_dir = os.path.dirname(module_path)

print(f"\nUsing module: {os.path.relpath(module_path, build_dir)}")
print(f"Module directory: {os.path.relpath(module_dir, build_dir)}")

# Add to path
if module_dir not in sys.path:
    sys.path.insert(0, module_dir)
    print(f"\nAdded to Python path: {module_dir}")

# Try to import
print("\n=== Testing import ===")
try:
    import rosetta_functions as rf
    print("✓ Successfully imported rosetta_functions")
except ImportError as e:
    print(f"❌ Failed to import: {e}")
    print("\nTry running from the module directory:")
    print(f"  cd {os.path.relpath(module_dir, build_dir)}")
    print(f"  python {os.path.join('..', 'test.py')}")
    sys.exit(1)

# Run tests
print("\n=== Running Tests ===\n")

try:
    # Test 1: add
    print("1. Testing add(20, 3):")
    result = rf.add(20, 3)
    print(f"   Result: {result}")
    assert result == 23, f"Expected 23, got {result}"
    print("   ✓ PASS\n")

    # Test 2: greet
    print("2. Testing greet('World'):")
    result = rf.greet("World")
    print(f"   Result: {result}")
    assert result == "Hello, World", f"Expected 'Hello, World', got '{result}'"
    print("   ✓ PASS\n")

    # Test 3: calculateDistance
    print("3. Testing calculateDistance(0, 0, 3, 4):")
    result = rf.calculateDistance(0, 0, 3, 4)
    print(f"   Result: {result}")
    assert abs(result - 5.0) < 0.001, f"Expected 5.0, got {result}"
    print("   ✓ PASS\n")

    # Test 4: normalize
    print("4. Testing normalize([1, 2, 3, 4]):")
    result = rf.normalize([1.0, 2.0, 3.0, 4.0])
    print(f"   Result: {result}")
    expected_sum = sum(result)
    assert abs(expected_sum - 1.0) < 0.001, f"Expected sum=1.0, got {expected_sum}"
    print(f"   Sum: {expected_sum}")
    print("   ✓ PASS\n")

    print("=== All Tests Completed Successfully! ===\n")
    
    # Print helpful info
    print("To run tests again:")
    print(f"  cd {os.path.relpath(module_dir, build_dir)}")
    print(f"  python ..\\test.py")
    print("\nOr set PYTHONPATH:")
    print(f"  set PYTHONPATH={module_dir}")
    print(f"  python test.py")

except AssertionError as e:
    print(f"\n❌ Test failed: {e}")
    sys.exit(1)
except AttributeError as e:
    print(f"\n❌ Function not found: {e}")
    print("\nAvailable functions:")
    for attr in dir(rf):
        if not attr.startswith('_'):
            print(f"  - {attr}")
    sys.exit(1)
except Exception as e:
    print(f"\n❌ Error: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)