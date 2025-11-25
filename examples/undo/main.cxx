#include <rosetta/rosetta.h>
#include <rosetta/extensions/undo_redo/manager.h>

class Point {
private:
    double x_ = 0.0;
    double y_ = 0.0;

public:
    Point() = default;
    Point(double x, double y) : x_(x), y_(y) {}

    double getX() const { return x_; }
    double getY() const { return y_; }
    void   setX(double x) { x_ = x; }
    void   setY(double y) { y_ = y; }

    void print() const { std::cout << "(" << x_ << ", " << y_ << ")"; }
};

class Shape {
private:
    std::string name_ = "Unnamed";
    Point       position_;
    double      rotation_ = 0.0;
    bool        visible_  = true;
    std::string color_    = "white";

public:
    // Getters
    const std::string &getName() const { return name_; }
    const Point       &getPosition() const { return position_; }
    double             getRotation() const { return rotation_; }
    bool               isVisible() const { return visible_; }
    const std::string &getColor() const { return color_; }

    // Setters
    void setName(const std::string &name) { name_ = name; }
    void setPosition(const Point &pos) { position_ = pos; }
    void setRotation(double rotation) { rotation_ = rotation; }
    void setVisible(bool visible) { visible_ = visible; }
    void setColor(const std::string &color) { color_ = color; }

    void print() const {
        std::cout << "Shape '" << name_ << "': pos=";
        position_.print();
        std::cout << ", rotation=" << rotation_ << ", visible=" << std::boolalpha << visible_
                  << ", color=" << color_ << "\n";
    }
};

// ============================================================================
// Register Classes with Rosetta
// ============================================================================

void register_classes() {
    using namespace rosetta::core;

    // Register Point
    ROSETTA_REGISTER_CLASS(Point)
        .property("x", &Point::getX, &Point::setX)
        .property("y", &Point::getY, &Point::setY);

    // Register Shape
    ROSETTA_REGISTER_CLASS(Shape)
        .property("name", &Shape::getName, &Shape::setName)
        .property("position", &Shape::getPosition, &Shape::setPosition)
        .property("rotation", &Shape::getRotation, &Shape::setRotation)
        .property("visible", &Shape::isVisible, &Shape::setVisible)
        .property("color", &Shape::getColor, &Shape::setColor);
}

void demo_field_level_undo_redo() {
    using namespace rosetta::core;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  DEMO 1: Field-Level Undo/Redo                       ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n\n";

    Shape           shape;
    UndoRedoManager manager;

    std::cout << "Initial state:\n";
    shape.print();

    std::cout << "\nApplying changes...\n";
    manager.applyChange(&shape, "name", std::string("Circle"), "Set name to Circle");
    shape.print();

    manager.applyChange(&shape, "color", std::string("red"), "Set color to red");
    shape.print();

    manager.applyChange(&shape, "rotation", 45.0, "Rotate to 45 degrees");
    shape.print();

    manager.applyChange(&shape, "visible", false, "Hide shape");
    shape.print();

    manager.printHistory();

    std::cout << "Undoing all changes...\n";
    for (int i = 0; i < 4; ++i) {
        manager.undo();
        shape.print();
    }

    manager.printHistory();

    std::cout << "Redoing 2 changes...\n";
    manager.redo();
    shape.print();
    manager.redo();
    shape.print();
}

void demo_snapshot_undo_redo() {
    using namespace rosetta::core;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  DEMO 2: Snapshot-Based Undo/Redo (Memento)          ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n\n";

    Shape          shape;
    HistoryManager history;

    std::cout << "Initial state:\n";
    shape.print();
    history.saveState(&shape, "Initial state");

    std::cout << "\nConfiguration 1: Red circle at (10, 20)\n";
    shape.setName("Circle");
    shape.setColor("red");
    shape.setPosition(Point(10, 20));
    shape.print();
    history.saveState(&shape, "Red circle config");

    std::cout << "\nConfiguration 2: Blue square at (50, 50), rotated\n";
    shape.setName("Square");
    shape.setColor("blue");
    shape.setPosition(Point(50, 50));
    shape.setRotation(30.0);
    shape.print();
    history.saveState(&shape, "Blue square config");

    std::cout << "\nConfiguration 3: Green triangle, hidden\n";
    shape.setName("Triangle");
    shape.setColor("green");
    shape.setVisible(false);
    shape.print();
    history.saveState(&shape, "Green triangle config");

    history.printHistory();

    std::cout << "Traveling back in history...\n";
    history.undo(&shape);
    shape.print();

    history.undo(&shape);
    shape.print();

    history.undo(&shape);
    shape.print();

    history.printHistory();

    std::cout << "Moving forward again...\n";
    history.redo(&shape);
    shape.print();

    history.redo(&shape);
    shape.print();
}

void demo_complex_object_undo() {
    using namespace rosetta::core;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  DEMO 3: Undo/Redo with Nested Objects               ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n\n";

    Shape           shape;
    UndoRedoManager manager;

    std::cout << "Initial state:\n";
    shape.print();

    std::cout << "\nChanging nested Point object:\n";
    Point new_pos(100, 200);
    manager.applyChange(&shape, "position", new_pos, "Move to (100, 200)");
    shape.print();

    Point another_pos(300, 400);
    manager.applyChange(&shape, "position", another_pos, "Move to (300, 400)");
    shape.print();

    std::cout << "\nUndo last position change:\n";
    manager.undo();
    shape.print();

    std::cout << "\nUndo again:\n";
    manager.undo();
    shape.print();
}

// ============================================================================
// Main
// ============================================================================

int main() {
    // Register all classes with reflection
    register_classes();

    // Run demos
    demo_field_level_undo_redo();
    demo_snapshot_undo_redo();
    demo_complex_object_undo();

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  All demos completed successfully!                   ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";

    return 0;
}