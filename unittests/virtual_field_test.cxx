/*
============================================================================
Complete Example: Virtual Field Registration
============================================================================

BENEFITS OF VIRTUAL FIELDS:

1. **Encapsulation**: Keep members private while still exposing them for introspection
   - Model::surfaces_ can remain private
   - Still accessible via metadata as "surfaces" field

2. **Validation**: Add validation in setters without changing the API
   - Setter can validate values before storing
   - Introspection code doesn't need to know about validation

3. **Computed Properties**: Expose calculated values as if they were fields
   - Rectangle::area is computed from width * height
   - Appears as a field in introspection
   - Can be read-only if it doesn't make sense to set it

4. **Consistency**: Uniform API for both real and virtual fields
   - Same get_field() / set_field() API
   - Same appears in fields() list
   - Binding generators treat them identically

5. **Non-intrusive**: No need to make members public or add friend declarations
   - Just need public getter/setter methods
   - Existing code doesn't need to change

6. **Flexibility**: Can change implementation without breaking introspection
   - Can switch from direct member to virtual field
   - Or vice versa
   - Metadata users won't notice the difference
*/
// ============================================================================

#include "TEST.h"

// Computed property (doesn't exist as a member)
class Rectangle {
private:
    double width_;
    double height_;

public:
    Rectangle(double w = 0, double h = 0) : width_(w), height_(h) {}

    // Regular fields
    const double &getWidth() const { return width_; }
    void          setWidth(const double &w) { width_ = w; }

    const double &getHeight() const { return height_; }
    void          setHeight(const double &h) { height_ = h; }

    // Computed property - area doesn't exist as a member!
    double getArea() const { return width_ * height_; }
    // No setArea - it's computed from width and height

    void                       run1(double) {}
    double                     run2(double, int) {return 0.;}
    const std::vector<double> &run3(double, int, bool) {return {};}
    bool                       run4(double, int, bool, const std::vector<double> &) {return false;}
    std::string                run5(const std::string &) {return "";}
};

// ============================================================================
// Registration
// ============================================================================

void initFct() {
    using namespace rosetta;

    // Register Rectangle with both real and computed properties
    ROSETTA_REGISTER_CLASS(Rectangle)
        .constructor<double, double>()
        // Virtual properties for width and height
        .property<double>("width", &Rectangle::getWidth, &Rectangle::setWidth)
        .property<double>("height", &Rectangle::getHeight, &Rectangle::setHeight)
        // Read-only computed property for area
        .readonly_property<double>("area", &Rectangle::getArea)
        .method("run1", &Rectangle::run1)
        .method("run2", &Rectangle::run2)
        .method("run3", &Rectangle::run3)
        .method("run4", &Rectangle::run4)
        .method("run5", &Rectangle::run5);
}

// ============================================================================
// Usage Examples
// ============================================================================

TEST(virtual_field, basic) {
    using namespace rosetta;

    Rectangle rect(5.0, 3.0);
    auto     &meta = ROSETTA_GET_META(Rectangle);

    // Get width (virtual field)
    auto width = meta.get_field(rect, "width").as<double>();
    std::cout << "Width: " << width << "\n"; // Outputs: 5.0

    // Set height (virtual field)
    meta.set_field(rect, "height", Any(4.0));

    // Get computed area (read-only virtual field)
    auto area = meta.get_field(rect, "area").as<double>();
    std::cout << "Area: " << area << "\n"; // Outputs: 20.0 (5.0 * 4.0)

    // This would throw an exception:
    // meta.set_field(rect, "area", Any(100.0));  // Error: read-only!

    meta.dump(std::cout);
}

RUN_TESTS()
