# REST API Generator Example

This example demonstrates how to generate a REST API server using the `binding_generator` tool.

## 1. Your C++ Classes

```cpp
// include/geometry/Vector3.h
#pragma once
#include <cmath>

namespace geometry {

class Vector3 {
public:
    double x = 0, y = 0, z = 0;

    Vector3() = default;
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    void normalize() {
        double len = length();
        if (len > 0) { x /= len; y /= len; z /= len; }
    }

    double dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};

} // namespace geometry
```

```cpp
// include/geometry/Sphere.h
#pragma once
#include "Vector3.h"
#include <cmath>

namespace geometry {

class Sphere {
public:
    Vector3 center;
    double radius = 1.0;

    Sphere() = default;
    Sphere(double r) : radius(r) {}

    double volume() const {
        return (4.0 / 3.0) * M_PI * radius * radius * radius;
    }

    double surface_area() const {
        return 4.0 * M_PI * radius * radius;
    }
};

} // namespace geometry
```

## 2. Rosetta Registration

```cpp
// src/registration.h
#pragma once
#include <rosetta/rosetta.h>
#include <geometry/Vector3.h>
#include <geometry/Sphere.h>

namespace geometry_rosetta {

inline void register_classes() {
    using namespace geometry;

    ROSETTA_REGISTER_CLASS(Vector3)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3::x)
        .field("y", &Vector3::y)
        .field("z", &Vector3::z)
        .method("length", &Vector3::length)
        .method("normalize", &Vector3::normalize)
        .method("dot", &Vector3::dot);

    ROSETTA_REGISTER_CLASS(Sphere)
        .constructor<>()
        .constructor<double>()
        .field("radius", &Sphere::radius)
        .method("volume", &Sphere::volume)
        .method("surface_area", &Sphere::surface_area);
}

} // namespace geometry_rosetta
```

## 3. Project Configuration

Create `project.json`:

```json
{
    "project": {
        "name": "geometry",
        "version": "1.0.0",
        "description": "Geometry library REST API",
        "author": "Your Name",
        "license": "MIT"
    },
    "rosetta": {
        "registration_header": "src/registration.h",
        "registration_namespace": "geometry_rosetta",
        "registration_function": "register_classes",
        "types_namespace": "geometry"
    },
    "includes": {
        "directories": [
            "include",
            "../rosetta/include"
        ],
        "headers": [
            "geometry/Vector3.h",
            "geometry/Sphere.h"
        ],
        "libraries": []
    },
    "output": {
        "base_dir": "./generated"
    },
    "targets": {
        "python": {
            "enabled": false
        },
        "wasm": {
            "enabled": false
        },
        "javascript": {
            "enabled": false
        },
        "rest": {
            "enabled": true
        }
    },
    "options": {
        "generate_stubs": false,
        "generate_typescript": false,
        "generate_readme": true,
        "generate_cmake": true
    }
}
```

## 4. Create Custom Generator Binary

Since the binding_generator needs to link with your Rosetta registration, create a custom main:

```cpp
// my_generator.cpp
#include "BindingGeneratorLib.h"
#include "src/registration.h"

int main(int argc, char* argv[]) {
    // Register your classes with Rosetta
    geometry_rosetta::register_classes();
    
    // Run the generator
    return BindingGeneratorLib::run(argc, argv);
}
```

## 5. Build and Run the Generator

```bash
# Build your custom generator
g++ -std=c++20 \
    -I/path/to/binding_generator \
    -I/path/to/rosetta/include \
    -Iinclude \
    my_generator.cpp \
    -o my_generator

# Generate the REST API code
./my_generator project.json
```

Output:
```
Loading configuration: project.json
Configuration loaded:
  Project: geometry v1.0.0
  Output:  ./generated
  Targets: rest

[OK] REST API server: ./generated/rest

Generated targets:
  REST API:
    cd ./generated/rest
    mkdir build && cd build && cmake .. && make
    ./geometry_server --port 8080
```

## 6. Generated Files

```
generated/
└── rest/
    ├── generated_rest_api.cxx    # Complete REST server code
    ├── CMakeLists.txt            # Build configuration
    ├── registration.h            # Copy of your registration header
    └── README.md                 # API documentation
```

## 7. Build the REST Server

```bash
cd generated/rest
mkdir build && cd build
cmake ..
make
```

## 8. Run the Server

```bash
./geometry_server --port 8080
```

Output:
```
geometry REST API Server
Listening on 0.0.0.0:8080

Endpoints:
  GET  /health              - Health check
  GET  /api/classes         - List all classes
  GET  /api/classes/:name   - Get class info
  GET  /api/objects         - List all objects
  POST /api/objects/:class  - Create object
  POST /api/objects/:id/:method - Call method
  DELETE /api/objects/:id   - Delete object

Press Ctrl+C to stop.
```

---

## 9. API Usage Examples

### List Classes

```bash
curl http://localhost:8080/api/classes
```
```json
{
  "error": false,
  "data": ["Vector3", "Sphere"]
}
```

### Get Class Info

```bash
curl http://localhost:8080/api/classes/Vector3
```
```json
{
  "error": false,
  "data": {
    "name": "Vector3",
    "cpp_type": "geometry::Vector3",
    "constructors": [
      {"params": []},
      {"params": ["double", "double", "double"]}
    ],
    "methods": [
      {"name": "length", "return_type": "double", "params": [], "is_const": true},
      {"name": "normalize", "return_type": "void", "params": [], "is_const": false},
      {"name": "dot", "return_type": "double", "params": ["geometry::Vector3"], "is_const": true}
    ]
  }
}
```

### Create a Vector3

```bash
curl -X POST http://localhost:8080/api/objects/Vector3 \
  -H "Content-Type: application/json" \
  -d '[3.0, 4.0, 0.0]'
```
```json
{"error":false,"data":{"id":"Vector3_1","class":"Vector3"}}
```

### Get Vector Length

```bash
curl -X POST http://localhost:8080/api/objects/Vector3_1/length
```
```json
{"error":false,"data":5.0}
```

### Normalize Vector

```bash
curl -X POST http://localhost:8080/api/objects/Vector3_1/normalize
```
```json
{"error":false}
```

### Create a Sphere

```bash
curl -X POST http://localhost:8080/api/objects/Sphere \
  -H "Content-Type: application/json" \
  -d '[2.5]'
```
```json
{"error":false,"data":{"id":"Sphere_1","class":"Sphere"}}
```

### Get Sphere Volume

```bash
curl -X POST http://localhost:8080/api/objects/Sphere_1/volume
```
```json
{"error":false,"data":65.44984694978736}
```

### List All Objects

```bash
curl http://localhost:8080/api/objects
```
```json
{
  "error": false,
  "data": [
    {"id": "Vector3_1", "class": "Vector3"},
    {"id": "Sphere_1", "class": "Sphere"}
  ]
}
```

### Delete an Object

```bash
curl -X DELETE http://localhost:8080/api/objects/Vector3_1
```
```json
{"error":false}
```

---

## 10. Python Client Example

```python
import requests

API = 'http://localhost:8080/api'

# List available classes
classes = requests.get(f'{API}/classes').json()['data']
print(f"Available classes: {classes}")

# Create a vector (3, 4, 0)
res = requests.post(f'{API}/objects/Vector3', json=[3.0, 4.0, 0.0])
vec_id = res.json()['data']['id']
print(f"Created: {vec_id}")

# Get length
length = requests.post(f'{API}/objects/{vec_id}/length').json()['data']
print(f"Length: {length}")  # 5.0

# Normalize
requests.post(f'{API}/objects/{vec_id}/normalize')

# Length after normalize
length = requests.post(f'{API}/objects/{vec_id}/length').json()['data']
print(f"Length after normalize: {length}")  # 1.0

# Create sphere with radius 2.5
res = requests.post(f'{API}/objects/Sphere', json=[2.5])
sphere_id = res.json()['data']['id']

# Get volume
volume = requests.post(f'{API}/objects/{sphere_id}/volume').json()['data']
print(f"Sphere volume: {volume}")  # 65.45...

# Cleanup
requests.delete(f'{API}/objects/{vec_id}')
requests.delete(f'{API}/objects/{sphere_id}')
```

---

## 11. JavaScript Client Example

```javascript
const API = 'http://localhost:8080/api';

async function main() {
    // List classes
    const classes = await fetch(`${API}/classes`)
        .then(r => r.json())
        .then(j => j.data);
    console.log('Classes:', classes);

    // Create vector
    const vecRes = await fetch(`${API}/objects/Vector3`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify([3.0, 4.0, 0.0])
    }).then(r => r.json());
    const vecId = vecRes.data.id;
    console.log('Created:', vecId);

    // Get length
    const length = await fetch(`${API}/objects/${vecId}/length`, {
        method: 'POST'
    }).then(r => r.json()).then(j => j.data);
    console.log('Length:', length);  // 5.0

    // Normalize
    await fetch(`${API}/objects/${vecId}/normalize`, {method: 'POST'});

    // Delete
    await fetch(`${API}/objects/${vecId}`, {method: 'DELETE'});
}

main();
```

---

## 12. Complete Workflow Summary

```
+------------------------------------------------------------------+
|  1. Define C++ classes (geometry/Vector3.h, Sphere.h)            |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  2. Create Rosetta registration (src/registration.h)            |
|     ROSETTA_REGISTER_CLASS(Vector3)                              |
|         .constructor<double, double, double>()                   |
|         .method("length", &Vector3::length)                      |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  3. Configure project.json with "rest": {"enabled": true}        |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  4. Run: ./my_generator project.json                             |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  5. Generated files in ./generated/rest/                         |
|     - generated_rest_api.cxx                                     |
|     - CMakeLists.txt                                             |
|     - README.md                                                  |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  6. Build: cd generated/rest && cmake -B build && make           |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  7. Run: ./geometry_server --port 8080                           |
+------------------------------------------------------------------+
                              |
                              v
+------------------------------------------------------------------+
|  8. Use the API:                                                 |
|     curl -X POST http://localhost:8080/api/objects/Vector3 \     |
|       -d '[3,4,0]'                                               |
|     curl -X POST http://localhost:8080/api/objects/Vector3_1/    |
|       length                                                     |
+------------------------------------------------------------------+
```