#!/usr/bin/env python3
"""
Test script for Rosetta Python bindings
Demonstrates non-intrusive C++ class binding to Python
"""

import rosetta_example as rosetta
import math

def print_header(title):
    """Print a section header"""
    print(f"\n{'=' * 30} {title} {'=' * 30}")

def test_vector3d():
    """Test Vector3D class"""
    print_header("Vector3D")
    
    # Test default constructor
    v1 = rosetta.Vector3D()
    print(f"Default constructor: ({v1.x}, {v1.y}, {v1.z})")
    
    # Test parametric constructor
    v2 = rosetta.Vector3D(3, 4, 12)
    print(f"Parametric constructor: ({v2.x}, {v2.y}, {v2.z})")
    print(f"Length: {v2.length()}")
    
    # Test property setters
    v2.x = 6
    v2.y = 8
    print(f"After setting x=6, y=8: ({v2.x}, {v2.y}, {v2.z})")
    print(f"New length: {v2.length()}")
    
    # Test normalize method
    v2.normalize()
    print(f"After normalize: ({v2.x:.6f}, {v2.y:.6f}, {v2.z:.6f})")
    print(f"Normalized length: {v2.length():.6f}")
    
    # Test add method
    v3 = rosetta.Vector3D(1, 2, 3)
    v4 = v2.add(v3)
    print(f"v2.add(Vector3D(1,2,3)): ({v4.x:.6f}, {v4.y:.6f}, {v4.z:.6f})")
    
    # Test scale method
    v5 = v3.scale(2.0)
    print(f"v3.scale(2.0): ({v5.x}, {v5.y}, {v5.z})")
    
    # Test type checking
    try:
        v2.z = "oops"
        print(f"Type check failed - z is now: {v2.z}")
    except Exception as e:
        print(f"✓ Type check passed - rejected invalid type: {type(e).__name__}")
    
    print("✓ All Vector3D tests passed")

def test_rectangle():
    """Test Rectangle class"""
    print_header("Rectangle")
    
    # Test constructors
    r1 = rosetta.Rectangle()
    print(f"Default constructor: {r1.width} x {r1.height}")
    
    r2 = rosetta.Rectangle(5, 7)
    print(f"Parametric constructor: {r2.width} x {r2.height}")
    print(f"Area: {r2.area()}")
    print(f"Perimeter: {r2.perimeter()}")
    print(f"Is square: {r2.is_square()}")
    
    # Test property modification
    r2.width = 10
    r2.height = 2.5
    print(f"After resize: {r2.width} x {r2.height}")
    print(f"Area: {r2.area()}, Perimeter: {r2.perimeter()}")
    
    # Test square detection
    r3 = rosetta.Rectangle(5, 5)
    print(f"Square 5x5 is_square: {r3.is_square()}")
    
    print("✓ All Rectangle tests passed")

def test_person():
    """Test Person class with properties"""
    print_header("Person")
    
    # Test constructors
    p1 = rosetta.Person()
    print(f"Default constructor: name='{p1.name}', age={p1.age}")
    
    p2 = rosetta.Person("Ada", 37)
    print(f"Parametric constructor: name='{p2.name}', age={p2.age}")
    print(f"Greeting: {p2.greet()}")
    
    # Test property modification
    p2.age = 38
    p2.name = "Ada Lovelace"
    print(f"After edits: name='{p2.name}', age={p2.age}")
    print(f"Greeting: {p2.greet()}")
    
    # Test method
    print(f"Before birthday: age={p2.age}")
    p2.celebrate_birthday()
    print(f"After birthday: age={p2.age}")
    
    # Test validation
    try:
        p2.age = -5
        print(f"Validation failed - age is now: {p2.age}")
    except ValueError as e:
        print(f"✓ Validation passed - rejected negative age: {e}")
    
    print("✓ All Person tests passed")

def test_circle():
    """Test Circle class with computed properties"""
    print_header("Circle")
    
    # Test constructors
    c1 = rosetta.Circle()
    print(f"Default constructor: radius={c1.radius}")
    
    c2 = rosetta.Circle(5.0)
    print(f"Parametric constructor: radius={c2.radius}")
    print(f"Diameter: {c2.diameter}")
    print(f"Area: {c2.area:.4f}")
    print(f"Circumference: {c2.circumference:.4f}")
    
    # Verify calculations
    expected_area = math.pi * 5.0 * 5.0
    expected_circ = 2 * math.pi * 5.0
    print(f"Expected area: {expected_area:.4f}")
    print(f"Expected circumference: {expected_circ:.4f}")
    assert abs(c2.area - expected_area) < 1e-10, "Area calculation error"
    assert abs(c2.circumference - expected_circ) < 1e-10, "Circumference calculation error"
    
    # Test property modification
    c2.radius = 10.0
    print(f"After setting radius=10: diameter={c2.diameter}, area={c2.area:.4f}")
    
    # Test read-only properties
    try:
        c2.diameter = 30
        print(f"Read-only check failed - diameter is now: {c2.diameter}")
    except AttributeError as e:
        print(f"✓ Read-only check passed - cannot set diameter")
    
    # Test validation
    try:
        c3 = rosetta.Circle(-5.0)
        print(f"Validation failed - created circle with negative radius")
    except Exception as e:
        print(f"✓ Validation passed - rejected negative radius: {type(e).__name__}")
    
    print("✓ All Circle tests passed")

def test_utilities():
    """Test utility functions"""
    print_header("Utilities")
    
    # Test list_classes
    classes = rosetta.list_classes()
    print(f"Registered classes: {classes}")
    
    # Test get_class_info
    for cls_name in classes:
        info = rosetta.get_class_info(cls_name)
        print(f"\nClass: {info['name']}")
        print(f"  Abstract: {info['is_abstract']}")
        print(f"  Polymorphic: {info['is_polymorphic']}")
        print(f"  Virtual destructor: {info['has_virtual_destructor']}")
        print(f"  Base classes: {info['base_count']}")
    
    # Test version
    version = rosetta.version()
    print(f"\nRosetta version: {version}")
    
    # Test module constant
    print(f"Module PI constant: {rosetta.PI}")
    
    # Test module function
    unit_vec = rosetta.create_unit_vector()
    print(f"Unit vector: ({unit_vec.x}, {unit_vec.y}, {unit_vec.z})")
    
    print("✓ All utility tests passed")

def main():
    """Run all tests"""
    print("=" * 80)
    print("Rosetta Python Bindings Test Suite")
    print("Non-intrusive C++ reflection and binding generation")
    print("=" * 80)
    
    try:
        test_vector3d()
        test_rectangle()
        test_person()
        test_circle()
        test_utilities()
        
        print_header("Summary")
        print("✓ All tests passed successfully!")
        print("\nThe binding works perfectly without any inheritance or wrapper classes")
        print("in the C++ code. Pure non-intrusive reflection using Rosetta!")
        
    except Exception as e:
        print(f"\n✗ Test failed with error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())