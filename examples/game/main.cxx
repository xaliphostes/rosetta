#include <cmath>
#include <iostream>
#include <rosetta/introspectable.h>
#include <sstream>

// Define a Vector3D class
class Vector3D {
public:
    float x, y, z;

    Vector3D()
        : x(0)
        , y(0)
        , z(0)
    {
    }
    Vector3D(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    // Useful methods for demonstration
    float magnitude() const { return std::sqrt(x * x + y * y + z * z); }

    std::string toString() const
    {
        std::stringstream ss;
        ss << "(" << x << ", " << y << ", " << z << ")";
        return ss.str();
    }

    // Equality operator for comparison
    bool operator==(const Vector3D& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

// 1. Register the new type Vector3D
REGISTER_TYPE(Vector3D);

// 2. Example class that uses Vector3D as a member
class GameObject : public rosetta::Introspectable {
    INTROSPECTABLE(GameObject)

private:
    std::string name;
    Vector3D position;
    Vector3D velocity;
    float health;

public:
    GameObject()
        : name("Unknown")
        , position(0, 0, 0)
        , velocity(0, 0, 0)
        , health(100.0f)
    {
    }

    GameObject(const std::string& n, const Vector3D& pos)
        : name(n)
        , position(pos)
        , velocity(0, 0, 0)
        , health(100.0f)
    {
    }

    // Getters and setters
    std::string getName() const { return name; }
    void setName(const std::string& n) { name = n; }

    Vector3D getPosition() const { return position; }
    void setPosition(const Vector3D& pos) { position = pos; }

    Vector3D getVelocity() const { return velocity; }
    void setVelocity(const Vector3D& vel) { velocity = vel; }

    float getHealth() const { return health; }
    void setHealth(float h) { health = h; }

    // Methods using Vector3D
    void move(const Vector3D& delta)
    {
        position.x += delta.x;
        position.y += delta.y;
        position.z += delta.z;
    }

    void teleport(float x, float y, float z) { position = Vector3D(x, y, z); }

    float distanceFromOrigin() const { return position.magnitude(); }

    std::string getInfo() const
    {
        return name + " at " + position.toString() + " with " + std::to_string(health) + " health";
    }
};

// 3. Registration implementation
void GameObject::registerIntrospection(rosetta::TypeRegistrar<GameObject> reg)
{
    reg.member("name", &GameObject::name)
        .member("position", &GameObject::position) // Vector3D member
        .member("velocity", &GameObject::velocity) // Vector3D member
        .member("health", &GameObject::health)
        .method("getName", &GameObject::getName)
        .method("setName", &GameObject::setName)
        .method("getPosition", &GameObject::getPosition) // Returns Vector3D
        .method("setPosition", &GameObject::setPosition) // Takes Vector3D parameter
        .method("getVelocity", &GameObject::getVelocity)
        .method("setVelocity", &GameObject::setVelocity)
        .method("getHealth", &GameObject::getHealth)
        .method("setHealth", &GameObject::setHealth)
        .method("move", &GameObject::move) // Takes Vector3D parameter
        .method("teleport", &GameObject::teleport) // Takes 3 float parameters
        .method("distanceFromOrigin", &GameObject::distanceFromOrigin)
        .method("getInfo", &GameObject::getInfo);
}

// 4. Usage example
int main()
{
    GameObject player("Hero", Vector3D(10.0f, 5.0f, 0.0f));

    std::cout << "=== Vector3D Introspection Demo ===" << std::endl;

    // Print class info - will show Vector3D types
    player.printClassInfo();
    std::cout << std::endl;

    // Access Vector3D members
    std::cout << "=== Vector3D Member Access ===" << std::endl;
    player.printMemberValue("position"); // Will show Vector3D type
    player.printMemberValue("velocity");
    std::cout << std::endl;

    // Modify Vector3D members
    std::cout << "=== Vector3D Member Modification ===" << std::endl;
    Vector3D newPos(20.0f, 15.0f, 5.0f);
    player.setMemberValue("position", newPos);
    player.printMemberValue("position");
    std::cout << std::endl;

    // Call methods that return Vector3D
    std::cout << "=== Vector3D Method Calls ===" << std::endl;
    auto pos = player.callMethod("getPosition");
    auto position_value = std::any_cast<Vector3D>(pos);
    std::cout << "Position from method: " << position_value.toString() << std::endl;

    // Call methods that take Vector3D parameters
    Vector3D movement(5.0f, 0.0f, -2.0f);
    player.callMethod("move", std::vector<std::any> { movement });
    player.callMethod("getInfo"); // Print updated info

    // Call method with multiple parameters
    player.callMethod("teleport", std::vector<std::any> { 0.0f, 0.0f, 10.0f });

    auto final_pos = std::any_cast<Vector3D>(player.callMethod("getPosition"));
    std::cout << "Final position: " << final_pos.toString() << std::endl;

    return 0;
}
