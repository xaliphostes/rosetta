# Complete Include Examples for IDL Files

## Basic Example

```yaml
module:
  name: mylib
  version: "1.0.0"

includes:
  # System includes
  - vector
  - string
  - memory
  
  # Your project headers
  - mylib/Vector3D.h
  - mylib/DataContainer.h

classes:
  - name: Vector3D
    # ...
```

Generated output in `binding.cxx`:
```cpp
#include <iostream>
#include <rosetta/generators/js/js_generator.h>
#include <rosetta/generators/js/type_converters.h>
#include <rosetta/rosetta.h>

// User-specified includes
#include <vector>
#include <string>
#include <memory>
#include "mylib/Vector3D.h"
#include "mylib/DataContainer.h"
```

## Advanced Examples

### Example 1: Standard Library Only

```yaml
includes:
  - vector
  - map
  - optional
  - string
  - memory
  - algorithm
```

Generates:
```cpp
#include <vector>
#include <map>
#include <optional>
#include <string>
#include <memory>
#include <algorithm>
```

### Example 2: Project Headers Only

```yaml
includes:
  - src/Vector3D.h
  - src/Matrix4x4.h
  - include/mylib/types.hpp
```

Generates:
```cpp
#include "src/Vector3D.h"
#include "src/Matrix4x4.h"
#include "include/mylib/types.hpp"
```

### Example 3: Mixed System and Project Headers

```yaml
includes:
  # STL
  - vector
  - string
  - optional
  - memory
  
  # Third-party libraries (system-installed)
  - boost/shared_ptr.hpp   # Has .hpp -> quotes
  - eigen3/Eigen/Core      # Has / -> quotes
  
  # Your project
  - myproject/core/Vector3D.h
  - myproject/core/Matrix.h
  - myproject/utils/helpers.hpp
```

Generates:
```cpp
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include "boost/shared_ptr.hpp"
#include "eigen3/Eigen/Core"
#include "myproject/core/Vector3D.h"
#include "myproject/core/Matrix.h"
#include "myproject/utils/helpers.hpp"
```

### Example 4: Different Project Structures

#### Flat Structure
```yaml
includes:
  - Vector3D.h
  - DataContainer.h
  - helpers.h
```

#### Nested Structure
```yaml
includes:
  - include/mylib/core/Vector3D.h
  - include/mylib/core/Matrix.h
  - include/mylib/utils/helpers.h
```

#### Mixed Depth
```yaml
includes:
  - core/types.h           # Relative to project root
  - utils/helpers.hpp
  - Vector3D.h             # Same directory
```

## Include Format Rules

The system uses smart formatting:

| Include String         | Result                            | Reason                 |
| ---------------------- | --------------------------------- | ---------------------- |
| `vector`               | `#include <vector>`               | No `/` or `.h`/`.hpp`  |
| `string`               | `#include <string>`               | No `/` or `.h`/`.hpp`  |
| `iostream`             | `#include <iostream>`             | No `/` or `.h`/`.hpp`  |
| `Vector3D.h`           | `#include "Vector3D.h"`           | Has `.h` extension     |
| `types.hpp`            | `#include "types.hpp"`            | Has `.hpp` extension   |
| `mylib/Vector3D.h`     | `#include "mylib/Vector3D.h"`     | Has `/` path separator |
| `src/core/types.h`     | `#include "src/core/types.h"`     | Has `/` path separator |
| `boost/shared_ptr.hpp` | `#include "boost/shared_ptr.hpp"` | Has `.hpp` extension   |

## Common Patterns

### Pattern 1: Minimal (Let Rosetta Headers Handle It)

```yaml
includes:
  - mylib/MyClass.h
```

The Rosetta headers already include common STL headers.

### Pattern 2: Explicit STL Dependencies

```yaml
includes:
  # Only the STL types you actually use
  - vector      # for std::vector<T> fields
  - optional    # for std::optional<T> fields
  - string      # for std::string fields
  - map         # for std::map<K,V> fields
  - mylib/MyClass.h
```

### Pattern 3: Forward Declarations

If your headers use forward declarations:

```yaml
includes:
  - memory      # for std::shared_ptr, std::unique_ptr
  - mylib/forward_decls.h
  - mylib/MyClass.h
```

### Pattern 4: Namespace Organization

```yaml
includes:
  # Core types
  - mylib/core/Vector3D.h
  - mylib/core/Matrix.h
  
  # Utilities
  - mylib/utils/helpers.h
  
  # Containers
  - mylib/containers/DataContainer.h
```

## JSON Format

The same rules apply in JSON:

```json
{
  "includes": [
    "vector",
    "string",
    "optional",
    "mylib/Vector3D.h",
    "mylib/DataContainer.h"
  ]
}
```

## Best Practices

### ✅ DO

```yaml
includes:
  # System headers first, alphabetically
  - memory
  - optional
  - string
  - vector
  
  # Then project headers, by directory depth
  - mylib/Vector3D.h
  - mylib/Matrix.h
  - mylib/utils/helpers.h
```

### ❌ DON'T

```yaml
includes:
  # Don't use angle brackets or quotes (system handles this)
  - "<vector>"     # Wrong!
  - "\"Vector3D.h\""  # Wrong!
  
  # Don't include Rosetta headers (already included)
  - rosetta/rosetta.h  # Redundant
  
  # Don't include binding headers (already included)
  - rosetta/generators/js/js_generator.h  # Redundant
```

## Troubleshooting

### Issue: "File not found" during compilation

**Problem**: Include path is wrong

**Solution**: Make sure the path in your IDL file matches your actual file structure:

```yaml
# If your file is at: src/mylib/Vector3D.h
includes:
  - mylib/Vector3D.h  # Assuming src/ is in include path
  
# Or use full path from project root
includes:
  - src/mylib/Vector3D.h
```

### Issue: Wrong include format in generated code

**Check your IDL file**:
- System headers (STL) → Use simple names: `vector`, `string`
- Project headers → Use paths: `mylib/Vector3D.h` or `Vector3D.h`

### Issue: Duplicate includes

The system automatically handles duplicates - they'll only appear once in the generated code.

## Example: Real-World Project

```yaml
module:
  name: physics_engine
  version: "2.1.0"
  namespace: physics

includes:
  # Standard library
  - vector
  - string
  - memory
  - optional
  - cmath
  
  # Physics engine headers
  - physics/core/Vector3D.h
  - physics/core/Quaternion.h
  - physics/core/Transform.h
  - physics/collision/BoundingBox.h
  - physics/collision/Collider.h
  - physics/dynamics/RigidBody.h
  - physics/utils/MathHelpers.h

converters:
  - type: "std::vector<Vector3D>"
    kind: vector

classes:
  - name: Vector3D
    # ...
```

This generates clean, organized includes in the binding code.