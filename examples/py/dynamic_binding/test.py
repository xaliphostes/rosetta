#!/usr/bin/env python3
"""
Test script for dynamic container type binding
Demonstrates the new bind_*_type() functions
"""

import rosetta_example as db
import sys

def print_section(title):
    """Print a formatted section header"""
    print("\n" + "=" * 70)
    print(title)
    print("=" * 70)

def test_basic_types():
    """Test basic custom types"""
    print_section("1. Testing Basic Custom Types")
    
    # Test Vec3
    v = db.Vec3(1.0, 2.0, 3.0)
    print(f"Vec3: ({v.x}, {v.y}, {v.z})")
    
    # Test Color with different constructors
    c1 = db.Color()  # Default constructor
    print(f"Color (default): RGBA({c1.r}, {c1.g}, {c1.b}, {c1.a})")
    
    c2 = db.Color(255, 128, 64, 255)  # 4-argument constructor
    print(f"Color (4-arg): RGBA({c2.r}, {c2.g}, {c2.b}, {c2.a})")
    
    c3 = db.Color(255, 0, 0, 128)  # Semi-transparent red
    print(f"Color (semi-transparent): RGBA({c3.r}, {c3.g}, {c3.b}, {c3.a})")

def test_vector_type():
    """Test std::vector<T>"""
    print_section("2. Testing std::vector<Vec3>")
    
    scene = db.Scene()
    
    # Create positions
    positions = [
        db.Vec3(0.0, 0.0, 0.0),
        db.Vec3(1.0, 0.0, 0.0),
        db.Vec3(1.0, 1.0, 0.0),
        db.Vec3(0.0, 1.0, 0.0)
    ]
    
    scene.positions = positions
    print(f"Set {len(positions)} positions")
    print(f"Retrieved: {scene.getPositionCount()} positions")
    
    for i, pos in enumerate(scene.positions):
        print(f"  Position {i}: ({pos.x}, {pos.y}, {pos.z})")
    
    # Add another position
    scene.addPosition(db.Vec3(0.5, 0.5, 1.0))
    print(f"After adding one: {scene.getPositionCount()} positions")
    
    return scene

def test_map_type(scene):
    """Test std::map<K,V>"""
    print_section("3. Testing std::map<std::string, Color>")
    
    # Create materials with explicit 4-argument constructor
    materials = {
        "red": db.Color(255, 0, 0, 255),
        "green": db.Color(0, 255, 0, 255),
        "blue": db.Color(0, 0, 255, 255),
        "semi_transparent": db.Color(128, 128, 128, 128)
    }
    
    scene.materials = materials
    print(f"Set {len(materials)} materials")
    print(f"Retrieved: {scene.getMaterialCount()} materials")
    
    for name, color in scene.materials.items():
        print(f"  {name}: RGBA({color.r}, {color.g}, {color.b}, {color.a})")
    
    # Add another material
    scene.addMaterial("white", db.Color(255, 255, 255, 255))
    print(f"After adding one: {scene.getMaterialCount()} materials")

def test_set_type(scene):
    """Test std::set<T>"""
    print_section("4. Testing std::set<int>")
    
    visible_ids = {1, 3, 5, 7, 9}
    scene.visible_ids = visible_ids
    print(f"Set {len(visible_ids)} visible IDs: {sorted(visible_ids)}")
    print(f"Retrieved: {sorted(scene.visible_ids)}")
    print(f"Count: {scene.getVisibleIdCount()}")
    
    scene.addVisibleId(11)
    print(f"After adding 11: {sorted(scene.visible_ids)}")
    
    # Test adding duplicate (should not increase size)
    scene.addVisibleId(5)
    print(f"After adding duplicate 5: {sorted(scene.visible_ids)} (size: {scene.getVisibleIdCount()})")

def test_array_type(scene):
    """Test std::array<T,N>"""
    print_section("5. Testing std::array<double, 16> (4x4 matrix)")
    
    # Identity matrix
    identity_matrix = [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    ]
    
    scene.transform_matrix = identity_matrix
    print("Set identity matrix")
    
    retrieved = scene.transform_matrix
    print("Retrieved matrix (4x4):")
    for i in range(4):
        row = retrieved[i*4:(i+1)*4]
        print(f"  [{row[0]:5.1f}, {row[1]:5.1f}, {row[2]:5.1f}, {row[3]:5.1f}]")

def test_unordered_map_type(scene):
    """Test std::unordered_map<K,V>"""
    print_section("6. Testing std::unordered_map<int, std::string>")
    
    id_to_name = {
        100: "camera",
        101: "light1",
        102: "light2",
        103: "mesh1"
    }
    
    scene.id_to_name = id_to_name
    print(f"Set {len(id_to_name)} ID-to-name mappings")
    for obj_id, name in sorted(scene.id_to_name.items()):
        print(f"  ID {obj_id}: {name}")
    
    scene.registerObject(104, "mesh2")
    print(f"After registering mesh2: {len(scene.id_to_name)} objects")

def test_unordered_set_type(scene):
    """Test std::unordered_set<T>"""
    print_section("7. Testing std::unordered_set<std::string>")
    
    layers = {"geometry", "lights", "cameras"}
    scene.active_layers = layers
    print(f"Set {len(layers)} active layers: {sorted(layers)}")
    print(f"Retrieved: {sorted(scene.active_layers)}")
    
    scene.activateLayer("effects")
    print(f"After activating 'effects': {sorted(scene.active_layers)}")

def test_deque_type(scene):
    """Test std::deque<T>"""
    print_section("8. Testing std::deque<Vec3>")
    
    path = [
        db.Vec3(0.0, 0.0, 0.0),
        db.Vec3(1.0, 0.0, 0.0),
        db.Vec3(1.0, 1.0, 0.0)
    ]
    
    scene.path = path
    print(f"Set path with {len(path)} waypoints")
    print(f"Retrieved: {scene.getPathLength()} waypoints")
    
    for i, pos in enumerate(scene.path):
        print(f"  Waypoint {i}: ({pos.x}, {pos.y}, {pos.z})")
    
    scene.addToPath(db.Vec3(2.0, 2.0, 0.0))
    print(f"After adding waypoint: {scene.getPathLength()} waypoints")

def test_type_conversions():
    """Test Python native type conversions"""
    print_section("9. Testing Python Native Type Conversions")
    
    scene = db.Scene()
    
    # Python list -> std::vector
    positions_from_list = [db.Vec3(float(x), 0.0, 0.0) for x in range(5)]
    scene.positions = positions_from_list
    print(f"From Python list: {scene.getPositionCount()} positions")
    
    # Python dict -> std::map
    materials_from_dict = {
        "yellow": db.Color(255, 255, 0, 255),
        "cyan": db.Color(0, 255, 255, 255)
    }
    scene.materials = materials_from_dict
    print(f"From Python dict: {scene.getMaterialCount()} materials")
    
    # Python set -> std::set
    ids_from_set = {2, 4, 6, 8}
    scene.visible_ids = ids_from_set
    print(f"From Python set: {scene.getVisibleIdCount()} IDs")
    
    # Python list -> std::array (must have exact size)
    scene.transform_matrix = [0.0] * 16
    print("From Python list to array: Success")

def test_type_safety():
    """Test type safety and error handling"""
    print_section("10. Testing Type Safety")
    
    scene = db.Scene()
    
    # Test wrong array size
    try:
        scene.transform_matrix = [0.0] * 10  # Wrong size!
        print("ERROR: Should have failed with wrong array size")
    except Exception as e:
        print(f"✓ Correctly rejected wrong array size: {type(e).__name__}")
    
    # Test wrong map key type
    try:
        scene.id_to_name = {"not_an_int": "value"}  # Wrong key type!
        print("ERROR: Should have failed with wrong map key type")
    except Exception as e:
        print(f"✓ Correctly rejected wrong map key type: {type(e).__name__}")

def test_empty_containers():
    """Test empty container handling"""
    print_section("11. Testing Empty Containers")
    
    scene = db.Scene()
    
    scene.positions = []
    print(f"Empty vector: {scene.getPositionCount()} positions")
    
    scene.materials = {}
    print(f"Empty map: {scene.getMaterialCount()} materials")
    
    scene.visible_ids = set()
    print(f"Empty set: {scene.getVisibleIdCount()} IDs")

def main():
    """Run all tests"""
    print("=" * 70)
    print("Dynamic Container Type Binding Test Suite")
    print("=" * 70)
        
    try:
        test_basic_types()
        scene = test_vector_type()
        test_map_type(scene)
        test_set_type(scene)
        test_array_type(scene)
        test_unordered_map_type(scene)
        test_unordered_set_type(scene)
        test_deque_type(scene)
        test_type_conversions()
        test_type_safety()
        test_empty_containers()
        
        print("\n" + "=" * 70)
        print("✓ All tests completed successfully!")
        print("=" * 70)
        return 0
        
    except Exception as e:
        print(f"\n✗ Test failed with error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())