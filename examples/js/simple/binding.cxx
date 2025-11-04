// ============================================================================
// Example: Using the non-intrusive JavaScript binding generator
// ============================================================================

#include <rosetta/generators/js/napi_generator.h>
#include <rosetta/rosetta.h>

// ============================================================================
// Example C++ Classes - NO INHERITANCE REQUIRED!
// ============================================================================

class Vector3D {
public:
    double x, y, z;

    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    Vector3D add(const Vector3D &other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }
};

class Rectangle {
public:
    double width;
    double height;

    Rectangle() : width(0), height(0) {}
    Rectangle(double w, double h) : width(w), height(h) {}

    double area() const { return width * height; }
    double perimeter() const { return 2 * (width + height); }
};

// Example with properties (getter/setter pattern)
class Person {
private:
    std::string name_;
    int age_;

public:
    Person() : name_(""), age_(0) {}
    Person(const std::string &n, int a) : name_(n), age_(a) {}

    // Getters return by const reference (for string) or by value (for primitives)
    const std::string &getName() const { return name_; }
    int getAge() const { return age_; }

    // Setters take const reference
    void setName(const std::string &n) { name_ = n; }
    void setAge(const int &a) { age_ = a; }

    std::string greet() const {
        return "Hello, I'm " + name_ + " and I'm " + std::to_string(age_) + " years old";
    }
};

// ============================================================================
// Registration with Rosetta (done once, typically in a registration file)
// ============================================================================

void register_classes() {
    // Register Vector3D
    ROSETTA_REGISTER_CLASS(Vector3D)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize)
        .method("add", &Vector3D::add);

    // Register Rectangle with direct fields
    ROSETTA_REGISTER_CLASS(Rectangle)
        .constructor<>()
        .constructor<double, double>()
        .field("width", &Rectangle::width)
        .field("height", &Rectangle::height)
        .method("area", &Rectangle::area)
        .method("perimeter", &Rectangle::perimeter);

    // Register Person with properties (demonstrates getter/setter pattern)
    ROSETTA_REGISTER_CLASS(Person)
        .constructor<>()
        .constructor<const std::string &, int>()
        .property<std::string>("name", &Person::getName, &Person::setName)
        .property<int>("age", &Person::getAge, &Person::setAge)
        .method("greet", &Person::greet);
}

// ============================================================================
// Node.js Module Initialization
// ============================================================================

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    // Register classes with Rosetta (if not already done)
    register_classes();

    // Create the JavaScript binding generator
    rosetta::bindings::JsBindingGenerator generator(env, exports);

    // Bind classes - that's it!
    generator
        .bind_class<Vector3D>()
        .bind_class<Rectangle>()
        .bind_class<Person>()
        .add_utilities();

    return exports;
}

// Register the Node.js module
NODE_API_MODULE(mymodule, InitModule)

// ============================================================================
// JavaScript Usage Example (from Node.js)
// ============================================================================

/*
// In JavaScript/Node.js:

const mymodule = require('./build/Release/mymodule');

// Create a Vector3D
const v1 = new mymodule.Vector3D(3, 4, 5);
console.log('x:', v1.x);  // 3
console.log('y:', v1.y);  // 4
console.log('z:', v1.z);  // 5
console.log('length:', v1.length());  // 7.071...

// Modify fields
v1.x = 10;
v1.normalize();
console.log('After normalize:', v1.x, v1.y, v1.z);

// Create another vector and add
const v2 = new mymodule.Vector3D(1, 1, 1);
const v3 = v1.add(v2);
console.log('Sum:', v3.x, v3.y, v3.z);

// Create a Rectangle
const rect = new mymodule.Rectangle(10, 20);
console.log('Width:', rect.width);   // 10
console.log('Height:', rect.height); // 20
console.log('Area:', rect.area());   // 200

// Modify fields
rect.width = 15;
console.log('New area:', rect.area()); // 300

// Create a Person (demonstrates properties)
const person = new mymodule.Person("Alice", 30);
console.log('Name:', person.name);    // "Alice" (via getName)
console.log('Age:', person.age);      // 30 (via getAge)
console.log(person.greet());          // "Hello, I'm Alice and I'm 30 years old"

// Modify through properties
person.name = "Bob";                   // Calls setName
person.age = 25;                       // Calls setAge
console.log(person.greet());          // "Hello, I'm Bob and I'm 25 years old"

// List all available classes
console.log('Available classes:', mymodule.listClasses());
// Output: ['Vector3D', 'Rectangle', 'Person']
*/

// ============================================================================
// Alternative: Bind multiple classes at once
// ============================================================================

Napi::Object InitModuleSimpler(Napi::Env env, Napi::Object exports) {
    register_classes();

    rosetta::bindings::JsBindingGenerator generator(env, exports);
    
    // Bind multiple classes in one call
    rosetta::bindings::bind_classes<Vector3D, Rectangle, Person>(generator);
    
    generator.add_utilities();
    
    return exports;
}

// ============================================================================
// What makes this better than the old approach:
//
// 1. NO INHERITANCE: Classes don't need to inherit from Introspectable
// 2. NON-INTRUSIVE: Works with any class registered in Rosetta
// 3. SIMPLER: Just register with Rosetta, then bind - that's it!
// 4. AUTOMATIC: All fields, properties, and methods are automatically bound
// 5. TYPE-SAFE: Uses Rosetta's type information for conversions
// 6. MAINTAINABLE: Single source of truth in Rosetta registration
// 7. FLEXIBLE: Can bind private members via property accessors
//
// Old approach required:
// - Class inherits from Introspectable ❌
// - Complex ObjectWrapper machinery ❌
// - Manual type converter registration ❌
// - TypeInfo member variables ❌
//
// New approach requires:
// - Register class with Rosetta ✓
// - Call bind_class<T>() ✓
// - Done! ✓
// ============================================================================