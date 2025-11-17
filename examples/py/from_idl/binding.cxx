#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>

#include "Matrix4x4.h"
#include "Scene.h"
#include "Shape.h"
#include "Sphere.h"
#include "Transform.h"
#include "Vector3D.h"
#include "geometry_functions.h"
// -------------------------------------
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace mylib;

// =============================================================================
// Rosetta Definition (for all generators)
// =============================================================================
void register_pygeometry_classes() {
    // Register Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize)
        .method("dot", &Vector3D::dot)
        .method("cross", &Vector3D::cross)
        .method("to_array", &Vector3D::to_array);

    // Register Matrix4x4
    ROSETTA_REGISTER_CLASS(Matrix4x4)
        .constructor<>()
        .constructor<const std::array<double, 16> &>()
        .readonly_property("data", &Matrix4x4::getData)
        .method("identity", &Matrix4x4::identity)
        .method("multiply", &Matrix4x4::multiply)
        .method("transform", &Matrix4x4::transform)
        .method("transpose", &Matrix4x4::transpose)
        .method("inverse", &Matrix4x4::inverse);

    // Register Shape
    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area")
        .pure_virtual_method<double>("volume")
        .virtual_method("getName", &Shape::getName)
        .virtual_method("boundingBox", &Shape::boundingBox);

    // Register Sphere
    ROSETTA_REGISTER_CLASS(Sphere)
        .inherits_from<Shape>()
        .constructor<const Vector3D &, double>()
        .property("center", &Sphere::getCenter, &Sphere::setCenter)
        .property("radius", &Sphere::getRadius, &Sphere::setRadius)
        .override_method("area", &Sphere::area)
        .override_method("volume", &Sphere::volume)
        .override_method("getName", &Sphere::getName)
        .method("containsPoint", &Sphere::containsPoint);

    // Register Transform
    ROSETTA_REGISTER_CLASS(Transform)
        .constructor<>()
        .method("getPosition", &Transform::getPosition)
        .method("setPosition", &Transform::setPosition)
        .method("getRotation", &Transform::getRotation)
        .method("setRotation", &Transform::setRotation)
        .method("getScale", &Transform::getScale)
        .method("setScale_vector3d", static_cast<void (Transform::*)(const Vector3D &)>(&Transform::setScale))
        .method("setScale_double", static_cast<void (Transform::*)(double)>(&Transform::setScale))
        .method("toMatrix", &Transform::toMatrix)
        .method("transformPoint", &Transform::transformPoint)
        .method("transformDirection", &Transform::transformDirection)
        .auto_detect_properties();

    // Register Scene
    ROSETTA_REGISTER_CLASS(Scene)
        .constructor<>()
        .method("addShape", &Scene::addShape)
        .method("getShapes", &Scene::getShapes)
        .method("findShapeByName", &Scene::findShapeByName)
        .method("clear", &Scene::clear)
        .method("getTotalVolume", &Scene::getTotalVolume)
        .method("getObjectCount", &Scene::getObjectCount);
}

// =============================================================================
// Python Module Definition using Rosetta registration above
// =============================================================================
BEGIN_PY_MODULE(from_idl, "Complete geometry library with advanced features") {
    // Register all classes with Rosetta introspection
    register_pygeometry_classes();

    // Register type converters
    // (use Rosetta introspection)
    BIND_STD_VECTOR(Vector3D);
    BIND_STD_VECTOR(Transform);
    BIND_STD_MAP(std::string, Vector3D);
    BIND_STD_MAP(int, Transform);
    BIND_STD_ARRAY(double, 16);
    BIND_STD_SET(int);

    // Bind classes to Python
    // (use Rosetta introspection)
    BIND_PY_CLASS(Vector3D);
    BIND_PY_CLASS(Matrix4x4);
    BIND_PY_CLASS(Shape);
    BIND_PY_DERIVED_CLASS(Sphere, Shape);
    BIND_PY_CLASS(Transform);
    BIND_PY_CLASS(Scene);

    // These allow Python to pass raw pointers that get wrapped in shared_ptr
    // (use Rosetta introspection)
    BIND_SHARED_PTR(Shape);
    BIND_SHARED_PTR(Sphere);

    // Bind free functions
    // (use Rosetta introspection)
    BIND_FUNCTION(create_vector, "Create a new Vector3D");
    BIND_FUNCTION(distance, "Calculate distance between two points");
    BIND_FUNCTION(lerp, "Linear interpolation between two vectors");
    BIND_FUNCTION(create_sphere, "Create a new sphere");
    BIND_FUNCTION(load_scene, "Load a scene from file");

    // Add utility functions
    // (use Rosetta introspection)
    BIND_PY_UTILITIES();
}
END_PY_MODULE()
