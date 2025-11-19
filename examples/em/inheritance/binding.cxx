// ============================================================================
// Example: Using inheritance with the Emscripten binding generator
// Demonstrates: Base classes, derived classes, abstract classes, virtual methods
// ============================================================================

#include <cmath>
#include <rosetta/extensions/generators/em_generator.h>
#include <rosetta/rosetta.h>

#ifndef M_PI
#define M_PI 3.1415926
#endif

// ============================================================================
// Abstract Base Class - Shape
// ============================================================================

class Shape {
public:
    virtual ~Shape() = default;
    
    // Pure virtual methods
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual std::string name() const = 0;
    
    // Common method for all shapes
    std::string describe() const {
        return name() + " with area " + std::to_string(area());
    }
};

// ============================================================================
// Derived Class - Circle (from Shape)
// ============================================================================

class Circle2D : public Shape {
private:
    double radius_;
    
public:
    Circle2D() : radius_(1.0) {}
    Circle2D(double r) : radius_(r) {}
    
    // Getters/Setters
    double getRadius() const { return radius_; }
    void setRadius(double r) { radius_ = r; }
    
    // Override virtual methods
    double area() const override { 
        return M_PI * radius_ * radius_; 
    }
    
    double perimeter() const override { 
        return 2 * M_PI * radius_; 
    }
    
    std::string name() const override { 
        return "Circle"; 
    }
};

// ============================================================================
// Derived Class - Rectangle2D (from Shape)
// ============================================================================

class Rectangle2D : public Shape {
private:
    double width_;
    double height_;
    
public:
    Rectangle2D() : width_(1.0), height_(1.0) {}
    Rectangle2D(double w, double h) : width_(w), height_(h) {}
    
    // Getters/Setters
    double getWidth() const { return width_; }
    void setWidth(double w) { width_ = w; }
    double getHeight() const { return height_; }
    void setHeight(double h) { height_ = h; }
    
    // Override virtual methods
    double area() const override { 
        return width_ * height_; 
    }
    
    double perimeter() const override { 
        return 2 * (width_ + height_); 
    }
    
    std::string name() const override { 
        return "Rectangle"; 
    }
    
    bool isSquare() const {
        return std::abs(width_ - height_) < 1e-10;
    }
};

// ============================================================================
// Further Derived Class - Square (from Rectangle2D)
// ============================================================================

class Square : public Rectangle2D {
public:
    Square() : Rectangle2D(1.0, 1.0) {}
    Square(double side) : Rectangle2D(side, side) {}
    
    double getSide() const { return getWidth(); }
    void setSide(double s) { 
        setWidth(s); 
        setHeight(s); 
    }
    
    std::string name() const override { 
        return "Square"; 
    }
};

// ============================================================================
// Another Inheritance Chain - Animals
// ============================================================================

class Animal {
protected:
    std::string name_;
    int age_;
    
public:
    Animal() : name_("Unknown"), age_(0) {}
    Animal(const std::string& n, int a) : name_(n), age_(a) {}
    virtual ~Animal() = default;
    
    const std::string& getName() const { return name_; }
    void setName(const std::string& n) { name_ = n; }
    int getAge() const { return age_; }
    void setAge(int a) { age_ = a; }
    
    virtual std::string speak() const = 0;
    virtual std::string type() const = 0;
    
    std::string introduce() const {
        return "I'm " + name_ + ", a " + std::to_string(age_) + 
               " year old " + type() + ". " + speak();
    }
};

class Dog : public Animal {
private:
    std::string breed_;
    
public:
    Dog() : Animal("Buddy", 3), breed_("Mixed") {}
    Dog(const std::string& name, int age, const std::string& breed) 
        : Animal(name, age), breed_(breed) {}
    
    const std::string& getBreed() const { return breed_; }
    void setBreed(const std::string& b) { breed_ = b; }
    
    std::string speak() const override { 
        return "Woof!"; 
    }
    
    std::string type() const override { 
        return breed_ + " dog"; 
    }
    
    void fetch() {
        // Dogs can fetch!
    }
};

class Cat : public Animal {
private:
    bool indoor_;
    
public:
    Cat() : Animal("Whiskers", 2), indoor_(true) {}
    Cat(const std::string& name, int age, bool indoor) 
        : Animal(name, age), indoor_(indoor) {}
    
    bool isIndoor() const { return indoor_; }
    void setIndoor(bool i) { indoor_ = i; }
    
    std::string speak() const override { 
        return "Meow!"; 
    }
    
    std::string type() const override { 
        return indoor_ ? "indoor cat" : "outdoor cat"; 
    }
};

// ============================================================================
// Free Functions that work with inheritance
// ============================================================================

double totalArea(const Shape& s1, const Shape& s2) {
    return s1.area() + s2.area();
}

std::string makeConversation(const Animal& a1, const Animal& a2) {
    return a1.getName() + " says: " + a1.speak() + 
           " " + a2.getName() + " replies: " + a2.speak();
}

// ============================================================================
// Registration with Rosetta
// ============================================================================

void register_inheritance_classes() {
    // Register Shape (abstract base)
    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area")
        .pure_virtual_method<double>("perimeter")
        .pure_virtual_method<std::string>("name")
        .method("describe", &Shape::describe);

    // Register Circle2D
    ROSETTA_REGISTER_CLASS(Circle2D)
        .inherits_from<Shape>()
        .constructor<>()
        .constructor<double>()
        .property<double>("radius", &Circle2D::getRadius, &Circle2D::setRadius)
        .override_method("area", &Circle2D::area)
        .override_method("perimeter", &Circle2D::perimeter)
        .override_method("name", &Circle2D::name);

    // Register Rectangle2D
    ROSETTA_REGISTER_CLASS(Rectangle2D)
        .inherits_from<Shape>()
        .constructor<>()
        .constructor<double, double>()
        .property<double>("width", &Rectangle2D::getWidth, &Rectangle2D::setWidth)
        .property<double>("height", &Rectangle2D::getHeight, &Rectangle2D::setHeight)
        .override_method("area", &Rectangle2D::area)
        .override_method("perimeter", &Rectangle2D::perimeter)
        .override_method("name", &Rectangle2D::name)
        .method("isSquare", &Rectangle2D::isSquare);

    // Register Square (inherits from Rectangle2D which inherits from Shape)
    ROSETTA_REGISTER_CLASS(Square)
        .inherits_from<Rectangle2D>()
        .constructor<>()
        .constructor<double>()
        .property<double>("side", &Square::getSide, &Square::setSide)
        .override_method("name", &Square::name);

    // Register Animal (abstract base)
    ROSETTA_REGISTER_CLASS(Animal)
        .property<std::string>("name", &Animal::getName, &Animal::setName)
        .property<int>("age", &Animal::getAge, &Animal::setAge)
        .pure_virtual_method<std::string>("speak")
        .pure_virtual_method<std::string>("type")
        .method("introduce", &Animal::introduce);

    // Register Dog
    ROSETTA_REGISTER_CLASS(Dog)
        .inherits_from<Animal>()
        .constructor<>()
        .constructor<const std::string&, int, const std::string&>()
        .property<std::string>("breed", &Dog::getBreed, &Dog::setBreed)
        .override_method("speak", &Dog::speak)
        .override_method("type", &Dog::type)
        .method("fetch", &Dog::fetch);

    // Register Cat
    ROSETTA_REGISTER_CLASS(Cat)
        .inherits_from<Animal>()
        .constructor<>()
        .constructor<const std::string&, int, bool>()
        .property<bool>("indoor", &Cat::isIndoor, &Cat::setIndoor)
        .override_method("speak", &Cat::speak)
        .override_method("type", &Cat::type);
}

// ============================================================================
// Emscripten Module Definition with Inheritance
// ============================================================================

BEGIN_EM_MODULE(shapes) {
    register_inheritance_classes();

    BIND_EM_UTILITIES();

    // Bind abstract base class
    BIND_EM_ABSTRACT_CLASS(Shape);
    
    // Bind derived shape classes
    BIND_EM_DERIVED_CLASS(Circle2D, Shape,
                         std::tuple<>,           // default constructor
                         std::tuple<double>      // Circle2D(radius)
    );
    
    BIND_EM_DERIVED_CLASS(Rectangle2D, Shape,
                         std::tuple<>,                  // default
                         std::tuple<double, double>     // Rectangle2D(w, h)
    );
    
    // Square derives from Rectangle2D (two levels of inheritance)
    BIND_EM_DERIVED_CLASS(Square, Rectangle2D,
                         std::tuple<>,           // default
                         std::tuple<double>      // Square(side)
    );

    // Bind abstract Animal base
    BIND_EM_ABSTRACT_CLASS(Animal);
    
    // Bind derived animal classes
    BIND_EM_DERIVED_CLASS(Dog, Animal,
                         std::tuple<>,                                      // default
                         std::tuple<std::string, int, std::string>          // Dog(name, age, breed)
    );
    
    BIND_EM_DERIVED_CLASS(Cat, Animal,
                         std::tuple<>,                          // default
                         std::tuple<std::string, int, bool>     // Cat(name, age, indoor)
    );

    // Bind free functions that accept base class references
    BIND_EM_FUNCTION(totalArea);
    BIND_EM_FUNCTION(makeConversation);
}
END_EM_MODULE()
