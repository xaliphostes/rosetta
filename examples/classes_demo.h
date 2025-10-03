#include <rosetta/introspectable.h>

// Example classes using the introspection system
class Person : public rosetta::Introspectable {
    INTROSPECTABLE(Person)
public:
    Person();
    Person(const std::string& n, int a, double h);

    // Getters and setters
    std::string getName() const;
    void setName(const std::string& n);
    int getAge() const;
    void setAge(int a);
    double getHeight() const;
    void setHeight(double h);
    bool getIsActive() const;
    void setIsActive(bool active);

    // Methods
    void introduce() const;
    void celebrateBirthday();
    std::string getDescription() const;

private:
    std::string name;
    int age;
    double height;
    bool isActive;
};

void Person::registerIntrospection(rosetta::TypeRegistrar<Person> reg)
{
    reg.constructor<>()
        .constructor<const std::string&, int, double>()
        .member("name", &Person::name)
        .member("age", &Person::age)
        .member("height", &Person::height)
        .member("isActive", &Person::isActive)
        .method("getName", &Person::getName)
        .method("setName", &Person::setName)
        .method("getAge", &Person::getAge)
        .method("setAge", &Person::setAge)
        .method("getHeight", &Person::getHeight)
        .method("setHeight", &Person::setHeight)
        .method("getIsActive", &Person::getIsActive)
        .method("setIsActive", &Person::setIsActive)
        .method("introduce", &Person::introduce)
        .method("celebrateBirthday", &Person::celebrateBirthday)
        .method("getDescription", &Person::getDescription);
}

// Another example class
class Vehicle : public rosetta::Introspectable {
    INTROSPECTABLE(Vehicle)
public:
    Vehicle();
    Vehicle(const std::string& b, const std::string& m, int y);

    // Getters and setters
    std::string getBrand() const;
    void setBrand(const std::string& b);
    std::string getModel() const;
    void setModel(const std::string& m);
    int getYear() const;
    void setYear(int y);
    double getMileage() const;
    void setMileage(double m);
    bool getIsRunning() const;

    // Methods
    void start();
    void stop();
    void drive(double miles);
    std::string getInfo() const;

private:
    std::string brand;
    std::string model;
    int year;
    double mileage;
    bool isRunning;
};

void Vehicle::registerIntrospection(rosetta::TypeRegistrar<Vehicle> reg)
{
    reg.constructor<>()
        .constructor<const std::string&, const std::string&, int>()
        .member("brand", &Vehicle::brand)
        .member("model", &Vehicle::model)
        .member("year", &Vehicle::year)
        .member("mileage", &Vehicle::mileage)
        .member("isRunning", &Vehicle::isRunning)
        .method("getBrand", &Vehicle::getBrand)
        .method("setBrand", &Vehicle::setBrand)
        .method("getModel", &Vehicle::getModel)
        .method("setModel", &Vehicle::setModel)
        .method("getYear", &Vehicle::getYear)
        .method("setYear", &Vehicle::setYear)
        .method("getMileage", &Vehicle::getMileage)
        .method("setMileage", &Vehicle::setMileage)
        .method("getIsRunning", &Vehicle::getIsRunning)
        .method("start", &Vehicle::start)
        .method("stop", &Vehicle::stop)
        .method("drive", &Vehicle::drive)
        .method("getInfo", &Vehicle::getInfo);
}

// =====================================================

Person::Person()
    : name("")
    , age(0)
    , height(0.0)
    , isActive(true)
{
}

Person::Person(const std::string& n, int a, double h)
    : name(n)
    , age(a)
    , height(h)
    , isActive(true)
{
}

std::string Person::getName() const { return name; }

void Person::setName(const std::string& n) { name = n; }

int Person::getAge() const { return age; }

void Person::setAge(int a) { age = a; }

double Person::getHeight() const { return height; }

void Person::setHeight(double h) { height = h; }

bool Person::getIsActive() const { return isActive; }

void Person::setIsActive(bool active) { isActive = active; }

void Person::introduce() const
{
    std::cout << "Hi! I'm " << name << ", " << age << " years old, " << height << "m tall."
              << std::endl;
}

void Person::celebrateBirthday()
{
    age++;
    std::cout << "🎉 " << name << " is now " << age << " years old!" << std::endl;
}

std::string Person::getDescription() const
{
    return name + " (" + std::to_string(age) + " years, " + std::to_string(height) + "m, "
        + (isActive ? "active" : "inactive") + ")";
}

// -----------------------------------------------------

Vehicle::Vehicle()
    : brand("")
    , model("")
    , year(0)
    , mileage(0.0)
    , isRunning(false)
{
}

Vehicle::Vehicle(const std::string& b, const std::string& m, int y)
    : brand(b)
    , model(m)
    , year(y)
    , mileage(0.0)
    , isRunning(false)
{
}

// Getters and setters
std::string Vehicle::getBrand() const { return brand; }

void Vehicle::setBrand(const std::string& b) { brand = b; }

std::string Vehicle::getModel() const { return model; }

void Vehicle::setModel(const std::string& m) { model = m; }

int Vehicle::getYear() const { return year; }

void Vehicle::setYear(int y) { year = y; }

double Vehicle::getMileage() const { return mileage; }

void Vehicle::setMileage(double m) { mileage = m; }

bool Vehicle::getIsRunning() const { return isRunning; }

void Vehicle::start()
{
    isRunning = true;
    std::cout << brand << " " << model << " started!" << std::endl;
}

void Vehicle::stop()
{
    isRunning = false;
    std::cout << brand << " " << model << " stopped!" << std::endl;
}

void Vehicle::drive(double miles)
{
    if (isRunning) {
        mileage += miles;
        std::cout << "Drove " << miles << " miles. Total mileage: " << mileage << std::endl;
    } else {
        std::cout << "Can't drive - vehicle is not running!" << std::endl;
    }
}

std::string Vehicle::getInfo() const
{
    return brand + " " + model + " (" + std::to_string(year) + ") - " + std::to_string(mileage)
        + " miles";
}

// -----------------------------------------------------
