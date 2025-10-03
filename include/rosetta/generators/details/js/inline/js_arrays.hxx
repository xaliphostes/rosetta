/*

- Copyright (c) 2025-now fmaerten@gmail.com
- LGPL v3 license
-

*/
#include <array>
#include "../js_generator.h"
#include "../js_common.h"
#include <rosetta/type_registry.h>
#include <type_traits>
#include <typeinfo>

namespace rosetta {

    namespace detail {
        /**
         * @brief Generic array to JavaScript converter
         * @tparam ArrayType The array type (std::array<T,N> or alias)
         * @tparam ElementType The element type
         * @tparam N Size of the array
         */
        template <typename ArrayType, typename ElementType, size_t N>
        inline Napi::Value arrayToJs(Napi::Env env, const std::any& value)
        {
            try {
                const auto& arr = std::any_cast<const ArrayType&>(value);
                auto js_arr = Napi::Array::New(env, N);
                for (size_t i = 0; i < N; ++i) {
                    js_arr.Set(i, toNapiValue<ElementType>(env, arr[i]));
                }
                return js_arr;
            } catch (const std::bad_any_cast&) {
                // Fallback: try value copy instead of reference
                auto arr = std::any_cast<ArrayType>(value);
                auto js_arr = Napi::Array::New(env, N);
                for (size_t i = 0; i < N; ++i) {
                    js_arr.Set(i, toNapiValue<ElementType>(env, arr[i]));
                }
                return js_arr;
            }
        }

        /**
         * @brief Generic JavaScript to array converter (factored implementation)
         * @tparam ArrayType The array type (std::array<T,N> or alias)
         * @tparam ElementType The element type
         * @tparam N Size of the array
         */
        template <typename ArrayType, typename ElementType, size_t N>
        inline std::any jsToArray(const Napi::Value& js_val)
        {
            if (!js_val.IsArray()) {
                throw Napi::TypeError::New(js_val.Env(), "Expected array");
            }

            auto js_arr = js_val.As<Napi::Array>();
            if (js_arr.Length() != N) {
                throw Napi::TypeError::New(js_val.Env(),
                    "Expected array of length " + std::to_string(N) + ", got "
                        + std::to_string(js_arr.Length()));
            }

            ArrayType arr;
            for (uint32_t i = 0; i < N; ++i) {
                arr[i] = fromNapiValue<ElementType>(js_arr.Get(i));
            }

            return arr;
        }
    } // namespace detail

    // ============================================================================
    // Auto array registration - Public API
    // ============================================================================

    /**
     * @brief Register std::array type converter using automatic type name from typeid
     * @tparam T Element type of the array
     * @tparam N Size of the array
     * @param generator The JavaScript generator to register with
     */
    template <typename T, size_t N> inline void registerArrayType(JsGenerator& generator)
    {
        using ArrayType = std::array<T, N>;
        std::string typeName = typeid(ArrayType).name();

        generator.register_type_converter(
            typeName, detail::arrayToJs<ArrayType, T, N>, detail::jsToArray<ArrayType, T, N>);
    }

    /**
     * @brief Register type alias converter for std::array
     * @tparam AliasType The type alias (e.g., Vec3 = std::array<double, 3>)
     * @tparam ElementType The element type of the underlying array
     * @tparam N Size of the array
     * @param generator The JavaScript generator to register with
     */
    template <typename AliasType, typename ElementType, size_t N>
    inline void registerArrayAlias(JsGenerator& generator)
    {
        std::string aliasName = typeid(AliasType).name();

        generator.register_type_converter(aliasName, detail::arrayToJs<AliasType, ElementType, N>,
            detail::jsToArray<AliasType, ElementType, N>);
    }

    /**
     * @brief Register common fixed-size array types
     * @param generator The JavaScript generator to register with
     */
    inline void registerCommonArrayTypes(JsGenerator& generator)
    {
        // 2D vectors/points
        registerArrayType<double, 2>(generator);
        registerArrayType<float, 2>(generator);
        registerArrayType<int, 2>(generator);

        // 3D vectors/points / 2x2 sym matrices
        registerArrayType<double, 3>(generator);
        registerArrayType<float, 3>(generator);
        registerArrayType<int, 3>(generator);

        // 4D vectors (homogeneous coordinates, quaternions)
        registerArrayType<double, 4>(generator);
        registerArrayType<float, 4>(generator);

        // 3x3 sym matrices (flattened)
        registerArrayType<double, 6>(generator);
        registerArrayType<float, 6>(generator);

        // 3x3 matrices (flattened)
        registerArrayType<double, 9>(generator);
        registerArrayType<float, 9>(generator);

        // 4x4 matrices (flattened)
        registerArrayType<double, 16>(generator);
        registerArrayType<float, 16>(generator);

        // RGB colors
        registerArrayType<unsigned char, 3>(generator);
        registerArrayType<float, 3>(generator);

        // RGBA colors
        registerArrayType<unsigned char, 4>(generator);
        registerArrayType<float, 4>(generator);
    }

} // namespace rosetta

// ============================================================================
// USAGE GUIDE
// ============================================================================
/*

## BASIC USAGE WITH std::array:

```
#include <rosetta/generators/js.h>
#include <rosetta/generators/js_arrays.h>
#include <array>

class Vector3D : public rosetta::Introspectable {
    INTROSPECTABLE(Vector3D)
public:
    Vector3D() : data_{0, 0, 0} {}
    Vector3D(double x, double y, double z) : data_{x, y, z} {}

    std::array<double, 3> getData() const { return data_; }
    void setData(const std::array<double, 3>& d) { data_ = d; }

    double magnitude() const {
        return std::sqrt(data_[0]*data_[0] +
                       data_[1]*data_[1] +
                       data_[2]*data_[2]);
    }

private:
    std::array<double, 3> data_;
};

void Vector3D::registerIntrospection(rosetta::TypeRegistrar<Vector3D> reg) {
    reg.constructor<>()
       .constructor<double, double, double>()
       .method("getData", &Vector3D::getData)
       .method("setData", &Vector3D::setData)
       .method("magnitude", &Vector3D::magnitude);
}

// Binding
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);

    // Register std::array<double, 3> converter
    rosetta::registerArrayType<double, 3>(generator);

    // Bind class
    generator.bind_class<Vector3D>();

    return exports;
}

NODE_API_MODULE(vector3d, Init)
```

JavaScript Usage:
const addon = require(’./build/Release/vector3d’);

```
const v = new addon.Vector3D(1.0, 2.0, 3.0);
console.log(v.getData());     // [1, 2, 3]
console.log(v.magnitude());   // 3.741...

v.setData([4.0, 5.0, 6.0]);
console.log(v.getData());     // [4, 5, 6]
```

## TYPE ALIASES WITH std::array:

```
#include <rosetta/generators/js.h>
#include <rosetta/generators/js_arrays.h>

// Define type aliases
using Vec2 = std::array<double, 2>;
using Vec3 = std::array<double, 3>;
using Vec4 = std::array<double, 4>;
using Matrix3 = std::array<double, 9>;   // 3x3 flattened
using Matrix4 = std::array<double, 16>;  // 4x4 flattened
using Color3 = std::array<float, 3>;     // RGB
using Color4 = std::array<float, 4>;     // RGBA

// Register them
REGISTER_TYPE_ALIAS_MANGLED(Vec2);
REGISTER_TYPE_ALIAS_MANGLED(Vec3);
REGISTER_TYPE_ALIAS_MANGLED(Vec4);
REGISTER_TYPE_ALIAS_MANGLED(Matrix3);
REGISTER_TYPE_ALIAS_MANGLED(Matrix4);
REGISTER_TYPE_ALIAS_MANGLED(Color3);
REGISTER_TYPE_ALIAS_MANGLED(Color4);

class Transform : public rosetta::Introspectable {
    INTROSPECTABLE(Transform)
public:
    Transform() : position_{0,0,0}, rotation_{0,0,0,1} {}

    Vec3 getPosition() const { return position_; }
    void setPosition(const Vec3& p) { position_ = p; }

    Vec4 getRotation() const { return rotation_; }
    void setRotation(const Vec4& r) { rotation_ = r; }

    Matrix4 getMatrix() const;  // Returns transformation matrix

private:
    Vec3 position_;
    Vec4 rotation_;  // Quaternion
};

// Binding
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);

    // Register type aliases
    rosetta::registerArrayAlias<Vec2, double, 2>(generator);
    rosetta::registerArrayAlias<Vec3, double, 3>(generator);
    rosetta::registerArrayAlias<Vec4, double, 4>(generator);
    rosetta::registerArrayAlias<Matrix3, double, 9>(generator);
    rosetta::registerArrayAlias<Matrix4, double, 16>(generator);

    generator.bind_class<Transform>();

    return exports;
}
```

JavaScript Usage:
const t = new addon.Transform();

```
t.setPosition([10, 20, 30]);
console.log(t.getPosition());  // [10, 20, 30]

// Quaternion (x, y, z, w)
t.setRotation([0, 0, 0.707, 0.707]);
console.log(t.getRotation());  // [0, 0, 0.707, 0.707]

const m = t.getMatrix();
console.log(m.length);         // 16 (4x4 matrix)
```

## BATCH REGISTRATION:

```
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);

    // Option 1: Register all common types
    rosetta::registerCommonArrayTypes(generator);

    // Option 2: Register specific types
    rosetta::registerArrayType<double, 2>(generator);
    rosetta::registerArrayType<double, 3>(generator);
    rosetta::registerArrayType<float, 4>(generator);

    // Then bind classes
    generator.bind_classes<Vector3D, Transform, Mesh>();

    return exports;
}
```

## CUSTOM STRUCT ARRAYS:

For arrays of custom types:

```
struct Point {
    double x, y, z;
};
REGISTER_TYPE(Point);

using Points = std::array<Point, 10>;
REGISTER_TYPE_ALIAS_MANGLED(Points);

// In binding:
generator.register_type_converter(
    typeid(Point).name(),
    // C++ to JS
    [](Napi::Env env, const std::any& value) -> Napi::Value {
        auto p = std::any_cast<Point>(value);
        auto obj = Napi::Object::New(env);
        obj.Set("x", p.x);
        obj.Set("y", p.y);
        obj.Set("z", p.z);
        return obj;
    },
    // JS to C++
    [](const Napi::Value& js_val) -> std::any {
        auto obj = js_val.As<Napi::Object>();
        return Point{
            obj.Get("x").As<Napi::Number>().DoubleValue(),
            obj.Get("y").As<Napi::Number>().DoubleValue(),
            obj.Get("z").As<Napi::Number>().DoubleValue()
        };
    }
);

rosetta::registerArrayAlias<Points, Point, 10>(generator);
```

## NESTED ARRAYS:

```
using Mat3x3 = std::array<std::array<double, 3>, 3>;
REGISTER_TYPE_ALIAS_MANGLED(Mat3x3);

// Register inner array first
rosetta::registerArrayType<double, 3>(generator);

// Then register outer array
rosetta::registerArrayType<std::array<double, 3>, 3>(generator);

// Or use alias
rosetta::registerArrayAlias<Mat3x3, std::array<double, 3>, 3>(generator);
```

JavaScript Usage:
const mat = [
[1, 0, 0],
[0, 1, 0],
[0, 0, 1]
];
obj.setMatrix(mat);

## COMPARISON: std::vector vs std::array

std::vector:
- Dynamic size
- Heap allocated
- Runtime size checks
- More flexible

std::array:
- Fixed size (compile-time)
- Stack allocated (usually)
- Compile-time size checks
- Better performance for small, fixed-size data
- Size is part of the type

Use std::array for:
- 2D/3D points and vectors
- Colors (RGB, RGBA)
- Quaternions
- Small matrices (3x3, 4x4)
- Fixed-size buffers

Use std::vector for:
- Variable-length data
- Large collections
- Data that grows/shrinks

## COMMON USE CASES:

1. 3D Graphics:
   using Vec3 = std::array<float, 3>;
   using Vec4 = std::array<float, 4>;
   using Mat4 = std::array<float, 16>;
1. Colors:
   using RGB = std::array<uint8_t, 3>;
   using RGBA = std::array<uint8_t, 4>;
1. 2D Games:
   using Vec2 = std::array<float, 2>;
   using Rect = std::array<float, 4>;  // x, y, width, height
1. Physics:
   using Force = std::array<double, 3>;
   using Quaternion = std::array<double, 4>;
1. Image Processing:
   using Pixel = std::array<uint8_t, 4>;
   using Kernel3x3 = std::array<float, 9>;

## DEBUGGING:

```
#include <rosetta/type_registry.h>

void debug_arrays() {
    auto& registry = rosetta::TypeNameRegistry::instance();

    // Check if array type is registered
    std::cout << "array<double,3> registered: "
              << registry.is_registered<std::array<double, 3>>()
              << "\n";

    // Get type name
    std::cout << "array<double,3> name: "
              << registry.get_name<std::array<double, 3>>()
              << "\n";
}
```

## PERFORMANCE NOTES:

1. Stack vs Heap:
- std::array<T, N> is typically stack-allocated
- Very fast for small N (N < 100)
- Prefer over std::vector for small, fixed-size data
1. Copy Semantics:
- std::array is copied by value (like C arrays)
- Consider using const& for parameters
- Move semantics work efficiently
1. JavaScript Conversion:
- Fixed-size validation at runtime
- Type checking for each element
- Efficient for small arrays (< 100 elements)

## ERROR HANDLING:

JavaScript will throw if:
- Wrong array size provided
- Wrong element types
- Non-array passed where array expected

```
try {
    v.setData([1, 2]);  // Error: Expected 3 elements
} catch (e) {
    console.error(e.message);
    // "Expected array of length 3, got 2"
}
```

*/