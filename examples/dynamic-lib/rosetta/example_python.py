#!/usr/bin/env python3
# python (reflection) binding for the `space` shared library.
#
#   cd bindings/python && cmake -B build && cmake --build build && cd -
#   python3 example_python.py
#
# The module's rpath points at ../space/bin, so libspace.dylib (where every
# method body actually lives) is found at import time wherever you run from.

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "bindings", "python"))

import space  # noqa: E402

v = space.Vector3(3.0, 0.0, 4.0)
print("Vector3(3, 0, 4).length() =", v.length())          # 5.0 — from libspace
n = v.normalized()
print("           .normalized() =", (n.x, n.y, n.z))        # (0.6, 0.0, 0.8)

a = space.Vector3(1, 2, 3)
b = space.Vector3(4, 5, 6)
print("a.dot(b)   =", a.dot(b))                             # 32.0
c = a.cross(b)
print("a.cross(b) =", (c.x, c.y, c.z))                      # (-3.0, 6.0, -3.0)

box = space.BoundingBox(space.Vector3(0, 0, 0), space.Vector3(2, 2, 2))
ctr = box.center()
print("box.center()   =", (ctr.x, ctr.y, ctr.z))            # (1.0, 1.0, 1.0)
print("box.diagonal() =", round(box.diagonal(), 4))         # 3.4641
print("box.contains((1,1,1)) =", box.contains(space.Vector3(1, 1, 1)))

# Out-of-line annotation (Vector3.ann.json) surfaced as a docstring:
print("doc(Vector3.x) =", space.Vector3.x.__doc__)
