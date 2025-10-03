#include <iostream>
#include <unordered_map>
#include "../../classes_demo.h"

// Example usage
int main()
{
    Person person("Alice", 30, 1.65);

    std::cout << "=== Class Introspection Demo ===" << std::endl;

    // Print class information using member method
    person.printClassInfo();
    std::cout << std::endl;

    // Access members through introspection using member methods
    std::cout << "=== Member Access ===" << std::endl;
    person.printMemberValue("name");
    person.printMemberValue("age");
    person.printMemberValue("height");
    std::cout << std::endl;

    // Modify members through introspection using member methods
    std::cout << "=== Member Modification ===" << std::endl;
    person.setMemberValue("name", std::string("Bob"));
    person.setMemberValue("age", 25);
    person.printMemberValue("name");
    person.printMemberValue("age");
    std::cout << std::endl;

    // Call methods through introspection using member methods
    std::cout << "=== Method Invocation ===" << std::endl;

    auto desc = person.callMethod("getDescription");
    std::cout << "Description: " << std::any_cast<std::string>(desc) << std::endl;

    // Call method with parameters
    person.callMethod("setName", std::vector<std::any>{std::string("Charlie")});
    person.callMethod("introduce");

    person.callMethod("setNameAndAge", std::vector<std::any>{std::string("Toto"), 22});
    person.callMethod("introduce");

    person.callMethod("setNameAgeAndHeight", std::vector<std::any>{std::string("Toto"), 22, 1.74});
    person.callMethod("introduce");

    std::cout << std::endl;

    // Demonstrate utility methods
    std::cout << "=== Utility Methods ===" << std::endl;
    std::cout << "Class name: " << person.getClassName() << std::endl;
    std::cout << "Has 'name' member: " << (person.hasMember("name") ? "yes" : "no") << std::endl;
    std::cout << "Has 'weight' member: " << (person.hasMember("weight") ? "yes" : "no") << std::endl;
    std::cout << "Has 'introduce' method: " << (person.hasMethod("introduce") ? "yes" : "no") << std::endl;

    // Get all member and method names
    std::cout << "\nAll members: ";
    for (const auto &name : person.getMemberNames())
    {
        std::cout << name << " ";
    }
    std::cout << "\nAll methods: ";
    for (const auto &name : person.getMethodNames())
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    std::cout << person.toJSON() << std::endl;

    return 0;
}
