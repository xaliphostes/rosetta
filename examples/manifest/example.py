import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "bindings", "python", "build"))

import manifest

p = manifest.Person("Toto", 18, "xd678shg")
print(p.name)
print(p.age)
print(p.id)
print(p.greet("Hello"))
