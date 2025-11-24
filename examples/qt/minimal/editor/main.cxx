#include <QApplication>
#include <rosetta/extensions/qt/qt_property_editor.h>
#include <rosetta/rosetta.h>

// Simple class to demonstrate
struct Person {
    std::string name   = "Alice";
    int         age    = 30;
    double      height = 1.75;

    void birthday() { age++; }

    void setName(const std::string &n) { name = n; }
    void setAge(int a) { age = a; }
    void setAgeAndHeight(int a, double h) {
        age    = a;
        height = h;
    }
    void reset() {
        name   = "Unknown";
        age    = 0;
        height = 0.0;
    }
};

// Register with Rosetta
static auto reg = []() {
    rosetta_registry()
        .register_class<Person>("Person")
        .field("name", &Person::name)
        .field("age", &Person::age)
        .field("height", &Person::height)
        .method("birthday", &Person::birthday)
        .method("setAge", &Person::setAge)
        .method("setName", &Person::setName)
        .method("setAgeAndHeight", &Person::setAgeAndHeight)
        .method("reset", &Person::reset);
    return 0;
}();

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Person person;

    auto *inspector = new rosetta::qt::ObjectInspector<Person>(&person);
    inspector->setWindowTitle("Person Inspector");
    inspector->resize(300, 400);
    inspector->show();

    return app.exec();
}