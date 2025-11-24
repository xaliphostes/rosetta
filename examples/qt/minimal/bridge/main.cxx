#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <rosetta/extensions/qt/qml_bridge.h>
#include <rosetta/rosetta.h>

// Simple class to demonstrate
struct Person : public QObject {
    Q_OBJECT
public:
    std::string name = "Alice";
    int         age  = 30;

    void        birthday() { age++; }
    std::string greet() const { return "Hello, I'm " + name; }
};

// Register with Rosetta
static auto reg = []() {
    rosetta::core::Registry::instance()
        .register_class<Person>("Person")
        .field("name", &Person::name)
        .field("age", &Person::age)
        .method("birthday", &Person::birthday)
        .method("greet", &Person::greet);
    return 0;
}();

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Register QmlBridge as QML type
    qmlRegisterType<rosetta::qt::QmlBridge>("Rosetta", 1, 0, "QmlBridge");

    // Create object and expose to QML
    Person person;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("personObj", &person);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

#include "main.moc"