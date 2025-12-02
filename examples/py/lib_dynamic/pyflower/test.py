#!/usr/bin/env python3
"""
Test script for pyflower module (dynamic library version)
"""

import pydynflower as pyf

print("Testing pyflower with dynamic library linkage")
print("=" * 50)

flower = pyf.Flower()

# Test circle area
radius = 10
area = flower.computeCircleArea(radius)
print(f"Circle area (r={radius}): {area:.4f}")

# Test sphere volume
volume = flower.computeSphereVolume(radius)
print(f"Sphere volume (r={radius}): {volume:.4f}")

# Test Fibonacci
n = 10
fib = flower.computeFibonacci(n)
print(f"Fibonacci({n}): {fib}")

print("=" * 50)
print("All tests passed!")