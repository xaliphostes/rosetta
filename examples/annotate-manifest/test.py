#!/usr/bin/env python3
# Verifies that the out-of-line annotations (widget.ann.json, wired by the
# manifest's "annotations" field) reach the compiled pybind11 module — even
# though widget.h itself carries no annotations.
#
#   build the module first:
#     ./generator out
#     cmake -S out/python -B out/python/build && cmake --build out/python/build
#   then:
#     python3 test.py

import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
for p in (os.path.join(here, "out", "python"),
          os.path.join(here, "out", "python", "build")):
    sys.path.insert(0, p)

import widget

w = widget.Widget()

# doc{"The widget title"} -> pybind property docstring
assert widget.Widget.title.__doc__ == "The widget title", \
    f"doc not applied: {widget.Widget.title.__doc__!r}"

# readonly -> assignment raises
try:
    w.id = 5
    raise SystemExit("FAIL: 'id' should be read-only")
except AttributeError:
    pass

# range{0,100} -> out-of-range raises, in-range works
w.count = 50
assert w.count == 50
try:
    w.count = 999
    raise SystemExit("FAIL: 'count' should reject out-of-range values")
except (ValueError, TypeError):
    pass

# plain fields still read/write
w.title = "Hello"
w.mode = "fast"
assert w.title == "Hello" and w.mode == "fast"

print("test.py OK — doc, readonly and range from widget.ann.json all applied")
