#!/usr/bin/env python3
"""
Comprehensive test for Rosetta Python bindings
Tests object creation, method calls, and object passing
"""

import sys

# Assuming the module is called 'basic' and is in the Python path
try:
    import basic
except ImportError as e:
    print(f"ERROR: Could not import basic module: {e}")
    print("Make sure the module is compiled and in PYTHONPATH")
    sys.exit(1)

def test_vector3d():
    """Test Vector3D class"""
    print("=" * 60)
    print("Testing Vector3D")
    print("=" * 60)
    
    # Create vector
    v1 = basic.Vector3D()
    print(f"✓ Created Vector3D: {v1}")
    
    # Set values
    v1.x = 3.0
    v1.y = 4.0
    v1.z = 0.0
    print(f"✓ Set values: x={v1.x}, y={v1.y}, z={v1.z}")
    
    # Calculate length
    length = v1.length()
    print(f"✓ Length: {length}")
    assert abs(length - 5.0) < 0.001, f"Expected length 5.0, got {length}"
    
    # Normalize
    v1.normalize()
    print(f"✓ After normalize: {v1.to_string()}")
    normalized_length = v1.length()
    assert abs(normalized_length - 1.0) < 0.001, f"Expected normalized length 1.0, got {normalized_length}"
    
    # Test add method (this is where the crash was happening)
    # print(1)
    v2 = basic.Vector3D()
    print(2)
    v2.x = 1.0
    v2.y = 1.0
    v2.z = 1.0
    print(f"✓ Created second vector: {v2.to_string()}")
    
    # This should work now with register_extractors() enabled
    v3 = v1.add(v2)
    print(f"✓ Sum: {v3.to_string()}")
    
    print("✓ All Vector3D tests passed!\n")

def test_shapes():
    """Test Shape hierarchy"""
    print("=" * 60)
    print("Testing Shapes")
    print("=" * 60)
    
    # Test Circle
    circle = basic.Circle()
    circle.radius = 5.0
    area = circle.area()
    name = circle.name()
    print(f"✓ Circle - radius: {circle.radius}, area: {area}, name: {name}")
    assert abs(area - 78.5398) < 0.01, f"Expected area ~78.54, got {area}"
    
    # Test Rectangle
    rect = basic.Rectangle()
    rect.width = 3.0
    rect.height = 4.0
    area = rect.area()
    name = rect.name()
    print(f"✓ Rectangle - width: {rect.width}, height: {rect.height}, area: {area}, name: {name}")
    assert abs(area - 12.0) < 0.001, f"Expected area 12.0, got {area}"
    
    print("✓ All Shape tests passed!\n")

def test_person():
    """Test Person class"""
    print("=" * 60)
    print("Testing Person")
    print("=" * 60)
    
    person = basic.Person()
    person.name = "Alice"
    person.age = 30
    print(f"✓ Created person: {person.name}, age {person.age}")
    
    person.add_hobby("reading")
    person.add_hobby("hiking")
    print(f"✓ Added hobbies")
    
    intro = person.introduce()
    print(f"✓ Introduction: {intro}")
    
    hobbies = person.get_hobbies()
    print(f"✓ Hobbies: {hobbies}")
    assert len(hobbies) == 2, f"Expected 2 hobbies, got {len(hobbies)}"
    
    print("✓ All Person tests passed!\n")

def test_functions():
    """Test free functions"""
    print("=" * 60)
    print("Testing Free Functions")
    print("=" * 60)
    
    v1 = basic.Vector3D()
    v1.x = 0.0
    v1.y = 0.0
    v1.z = 0.0
    
    v2 = basic.Vector3D()
    v2.x = 3.0
    v2.y = 4.0
    v2.z = 0.0
    
    dist = basic.distance(v1, v2)
    print(f"✓ Distance between vectors: {dist}")
    assert abs(dist - 5.0) < 0.001, f"Expected distance 5.0, got {dist}"
    
    unit = basic.create_unit_vector(3.0, 4.0, 0.0)
    print(f"✓ Unit vector: {unit.to_string()}")
    unit_length = unit.length()
    assert abs(unit_length - 1.0) < 0.001, f"Expected unit length 1.0, got {unit_length}"
    
    print("✓ All function tests passed!\n")

def test_utilities():
    """Test utility functions"""
    print("=" * 60)
    print("Testing Utilities")
    print("=" * 60)
    
    version = basic.get_version()
    print(f"✓ Rosetta version: {version}")
    
    classes = basic.list_classes()
    print(f"✓ Available classes: {classes}")
    expected_classes = ["Vector3D", "Circle", "Rectangle", "Person", "Shape"]
    for cls in expected_classes:
        assert cls in classes, f"Expected class '{cls}' not found in registry"
    
    print("✓ All utility tests passed!\n")

def main():
    """Run all tests"""
    print("\n" + "=" * 60)
    print("ROSETTA PYTHON BINDINGS - COMPREHENSIVE TEST SUITE")
    print("=" * 60 + "\n")
    
    try:
        test_vector3d()
        test_shapes()
        test_person()
        test_functions()
        test_utilities()
        
        print("=" * 60)
        print("✓✓✓ ALL TESTS PASSED! ✓✓✓")
        print("=" * 60)
        return 0
        
    except AssertionError as e:
        print(f"\n✗ TEST FAILED: {e}")
        return 1
    except Exception as e:
        print(f"\n✗ UNEXPECTED ERROR: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())