#include <rosetta/info.h>
#include <rosetta/introspectable.h>

class Person : public rosetta::Introspectable // <--------- !!!
{
    INTROSPECTABLE(Person) // <--------- !!!
public:
    Person();
    Person(const std::string& name, int age, double height);

    void introduce();
    std::string getName() const;
    void setName(const std::string& n);
    int getAge() const;
    void setAge(int a);
    double getHeight() const;
    void setHeight(double h);
    void setNameAndAge(const std::string&, int);
    void setNameAgeAndHeight(const std::string&, int, double);
    std::string getDescription() const;

private:
    std::string name;
    int age;
    double height;
};

// =============================================

Person::Person()
    : name("")
    , age(0)
    , height(0.0)
{
}
Person::Person(const std::string& n, int a, double h)
    : name(n)
    , age(a)
    , height(h)
{
}

// Regular methods
void Person::introduce()
{
    std::cout << "Hello, I'm " << name << ", " << age << " years old, " << height << "m tall."
              << std::endl;
}

std::string Person::getName() const { return name; }
void Person::setName(const std::string& n) { name = n; }

int Person::getAge() const { return age; }
void Person::setAge(int a) { age = a; }

double Person::getHeight() const { return height; }
void Person::setHeight(double h) { height = h; }

void Person::setNameAndAge(const std::string& n, int a)
{
    setName(n);
    setAge(a);
}

void Person::setNameAgeAndHeight(const std::string& n, int a, double h)
{
    setNameAndAge(n, a);
    setHeight(h);
}

std::string Person::getDescription() const
{
    return name + " (" + std::to_string(age) + " years, " + std::to_string(height) + "m)";
}

/**
 * @brief Registration implementation using templates (inherited static method)
 */
void Person::registerIntrospection(rosetta::TypeRegistrar<Person> reg)
{
    reg.constructor<>() // Default constructor
        .constructor<const std::string&, int, double>() // Parameterized constructor
        .member("name", &Person::name)
        .member("age", &Person::age)
        .member("height", &Person::height)
        .method("introduce", &Person::introduce)
        .method("getName", &Person::getName)
        .method("setName", &Person::setName)
        .method("getAge", &Person::getAge)
        .method("setAge", &Person::setAge)
        .method("getHeight", &Person::getHeight)
        .method("setHeight", &Person::setHeight)
        .method("setNameAndAge", &Person::setNameAndAge)
        .method("setNameAgeAndHeight", &Person::setNameAgeAndHeight)
        .method("getDescription", &Person::getDescription);
}
