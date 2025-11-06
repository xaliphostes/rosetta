#!/bin/bash

# Definitive Rebuild Script for Rosetta Python Bindings
#
# This script ensures a complete clean rebuild

set -e  # Exit on error

echo "========================================="
echo "Rosetta Python Bindings - Clean Rebuild"
echo "========================================="

# Step 1: Navigate to build directory
cd "$(dirname "$0")"
PROJECT_DIR="$(pwd)"

echo ""
echo "Project directory: $PROJECT_DIR"
echo ""

# Step 2: Remove old build artifacts
echo "Step 1: Cleaning old build..."
if [ -d "build" ]; then
    rm -rf build/
    echo "✓ Removed old build directory"
else
    echo "✓ No old build directory found"
fi

# Step 3: Create fresh build directory
echo ""
echo "Step 2: Creating fresh build directory..."
mkdir -p build
cd build
echo "✓ Created build directory"

# Step 4: Run CMake
echo ""
echo "Step 3: Configuring with CMake..."
cmake .. || {
    echo "✗ CMake configuration failed!"
    exit 1
}
echo "✓ CMake configuration successful"

# Step 5: Build
echo ""
echo "Step 4: Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
    echo "✗ Build failed!"
    exit 1
}
echo "✓ Build successful"

# Step 6: Find the built module
echo ""
echo "Step 5: Locating built module..."
MODULE=$(find . -name "*.so" -o -name "*.pyd" | head -1)

if [ -z "$MODULE" ]; then
    echo "✗ Could not find built module (.so or .pyd file)"
    exit 1
fi

echo "✓ Found module: $MODULE"

# Step 7: Show module info
echo ""
echo "Module Information:"
echo "  Path: $(pwd)/$MODULE"
echo "  Size: $(du -h "$MODULE" | cut -f1)"
echo "  Modified: $(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$MODULE" 2>/dev/null || stat -c "%y" "$MODULE" 2>/dev/null | cut -d'.' -f1)"

# Step 8: Run tests
echo ""
echo "Step 6: Running tests..."
cd "$PROJECT_DIR"

if [ -f "test.py" ]; then
    python3 test.py || {
        echo "✗ Tests failed"
        exit 1
    }
    echo "✓ All tests passed!"
elif [ -f "build/test.py" ]; then
    cd build
    python3 test.py || {
        echo "✗ Tests failed"
        exit 1
    }
    echo "✓ All tests passed!"
else
    echo "⚠ test.py not found, skipping tests"
fi

echo ""
echo "========================================="
echo "✓ Rebuild complete and successful!"
echo "========================================="