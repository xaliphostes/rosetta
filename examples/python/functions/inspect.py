#!/usr/bin/env python3
"""
Script to inspect a .pyd file and discover its module name
"""
import sys
import os
import importlib.util
import glob

def inspect_pyd_file(pyd_path):
    """Inspect a .pyd file to find its module name"""
    
    print(f"\n{'='*60}")
    print(f"Inspecting: {pyd_path}")
    print(f"{'='*60}")
    
    # 1. File information
    if not os.path.exists(pyd_path):
        print(f"❌ File not found: {pyd_path}")
        return None
    
    file_size = os.path.getsize(pyd_path) / 1024  # KB
    print(f"\n📄 File Info:")
    print(f"   Size: {file_size:.1f} KB")
    print(f"   Full path: {os.path.abspath(pyd_path)}")
    
    # 2. Extract module name from filename
    filename = os.path.basename(pyd_path)
    print(f"\n📝 Filename: {filename}")
    
    # .pyd files are named: modulename.cpXX-platform.pyd
    # Example: rosetta_functions.cp313-win_arm64.pyd
    base_name = filename.split('.')[0]
    print(f"   Base name (module name): {base_name}")
    
    # 3. Try to load the module
    print(f"\n🔍 Attempting to load module...")
    
    module_dir = os.path.dirname(os.path.abspath(pyd_path))
    if module_dir not in sys.path:
        sys.path.insert(0, module_dir)
        print(f"   Added to path: {module_dir}")
    
    try:
        # Method 1: Import by name
        module = __import__(base_name)
        print(f"   ✓ Successfully imported as: {base_name}")
        
        # 4. Show module attributes
        print(f"\n📦 Module Information:")
        if hasattr(module, '__name__'):
            print(f"   Module name: {module.__name__}")
        if hasattr(module, '__doc__'):
            doc = module.__doc__ or "(no docstring)"
            print(f"   Docstring: {doc[:60]}...")
        if hasattr(module, '__file__'):
            print(f"   File: {module.__file__}")
        
        # 5. List exported functions/classes
        print(f"\n🔧 Exported Functions/Classes:")
        exports = [name for name in dir(module) if not name.startswith('_')]
        if exports:
            for name in sorted(exports):
                obj = getattr(module, name)
                obj_type = type(obj).__name__
                print(f"   - {name:30s} ({obj_type})")
        else:
            print("   (none found)")
        
        # 6. Show usage example
        print(f"\n💡 Usage:")
        print(f"   import {base_name}")
        if exports:
            print(f"   {base_name}.{exports[0]}(...)")
        
        return base_name
        
    except ImportError as e:
        print(f"   ❌ Failed to import: {e}")
        print(f"\n   The module name is likely: {base_name}")
        print(f"   But there may be missing dependencies.")
        return base_name
    
    except Exception as e:
        print(f"   ❌ Error: {e}")
        return base_name


def find_and_inspect_pyd_files(search_dir="."):
    """Find all .pyd files in directory and inspect them"""
    
    print("Searching for .pyd files...")
    pyd_files = glob.glob(os.path.join(search_dir, "**/*.pyd"), recursive=True)
    
    if not pyd_files:
        print(f"❌ No .pyd files found in {os.path.abspath(search_dir)}")
        print("\nSearching in current directory...")
        pyd_files = glob.glob("*.pyd")
    
    if not pyd_files:
        print("❌ No .pyd files found!")
        return []
    
    print(f"\n✓ Found {len(pyd_files)} .pyd file(s):\n")
    
    module_names = []
    for pyd_file in pyd_files:
        module_name = inspect_pyd_file(pyd_file)
        if module_name:
            module_names.append(module_name)
    
    return module_names


if __name__ == "__main__":
    print("="*60)
    print("Python Extension Module (.pyd) Inspector")
    print("="*60)
    
    if len(sys.argv) > 1:
        # Specific file provided
        pyd_path = sys.argv[1]
        inspect_pyd_file(pyd_path)
    else:
        # Search current directory
        module_names = find_and_inspect_pyd_files(".")
        
        if module_names:
            print("\n" + "="*60)
            print("SUMMARY")
            print("="*60)
            print("\nModule names found:")
            for name in module_names:
                print(f"  • {name}")
            print("\nTo use:")
            for name in module_names:
                print(f"  import {name}")