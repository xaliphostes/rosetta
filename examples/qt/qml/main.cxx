/**
 * Simple Rosetta QML Demo
 * Demonstrates dynamic property editing using QmlBridge
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <rosetta/extensions/qt/qml_bridge.h>
#include <rosetta/rosetta.h>

// ============================================================================
// Example Classes - Simple 2D shapes
// ============================================================================

class Shape : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(double x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(double y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(double rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit Shape(QObject *parent = nullptr) : QObject(parent) {}

    QString name() const { return name_; }
    void    setName(const QString &n) {
        if (name_ != n) {
            name_ = n;
            emit nameChanged();
        }
    }

    double x() const { return x_; }
    void   setX(double v) {
        if (x_ != v) {
            x_ = v;
            emit xChanged();
        }
    }

    double y() const { return y_; }
    void   setY(double v) {
        if (y_ != v) {
            y_ = v;
            emit yChanged();
        }
    }

    double rotation() const { return rotation_; }
    void   setRotation(double v) {
        if (rotation_ != v) {
            rotation_ = v;
            emit rotationChanged();
        }
    }

    bool visible() const { return visible_; }
    void setVisible(bool v) {
        if (visible_ != v) {
            visible_ = v;
            emit visibleChanged();
        }
    }

    Q_INVOKABLE void reset() {
        setX(0);
        setY(0);
        setRotation(0);
    }

    Q_INVOKABLE void moveBy(double dx, double dy) {
        setX(x_ + dx);
        setY(y_ + dy);
    }

signals:
    void nameChanged();
    void xChanged();
    void yChanged();
    void rotationChanged();
    void visibleChanged();

private:
    QString name_ = "Shape";
    double  x_ = 0, y_ = 0, rotation_ = 0;
    bool    visible_ = true;
};

class ShapeCircle : public Shape {
    Q_OBJECT
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit ShapeCircle(QObject *parent = nullptr) : Shape(parent) { setName("ShapeCircle"); }

    double radius() const { return radius_; }
    void   setRadius(double v) {
        if (radius_ != v) {
            radius_ = v;
            emit radiusChanged();
        }
    }

    QString color() const { return color_; }
    void    setColor(const QString &c) {
        if (color_ != c) {
            color_ = c;
            emit colorChanged();
        }
    }

    Q_INVOKABLE double area() const { return 3.14159 * radius_ * radius_; }

signals:
    void radiusChanged();
    void colorChanged();

private:
    double  radius_ = 50;
    QString color_  = "#ff6b6b";
};

class ShapeRectangle : public Shape {
    Q_OBJECT
    Q_PROPERTY(double width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(double height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit ShapeRectangle(QObject *parent = nullptr) : Shape(parent) { setName("ShapeRectangle"); }

    double width() const { return width_; }
    void   setWidth(double v) {
        if (width_ != v) {
            width_ = v;
            emit widthChanged();
        }
    }

    double height() const { return height_; }
    void   setHeight(double v) {
        if (height_ != v) {
            height_ = v;
            emit heightChanged();
        }
    }

    QString color() const { return color_; }
    void    setColor(const QString &c) {
        if (color_ != c) {
            color_ = c;
            emit colorChanged();
        }
    }

    Q_INVOKABLE double area() const { return width_ * height_; }
    Q_INVOKABLE void   setSize(double w, double h) {
        setWidth(w);
        setHeight(h);
    }

signals:
    void widthChanged();
    void heightChanged();
    void colorChanged();

private:
    double  width_ = 100, height_ = 60;
    QString color_ = "#4ecdc4";
};

// ============================================================================
// Rosetta Registration
// ============================================================================

void registerRosettaTypes() {
    using namespace rosetta::core;

    // Register Shape base class
    Registry::instance()
        .register_class<Shape>("Shape")
        .property("name", &Shape::name, &Shape::setName)
        .property("x", &Shape::x, &Shape::setX)
        .property("y", &Shape::y, &Shape::setY)
        .property("rotation", &Shape::rotation, &Shape::setRotation)
        .property("visible", &Shape::visible, &Shape::setVisible)
        .method("reset", &Shape::reset)
        .method("moveBy", &Shape::moveBy);

    // Register ShapeCircle with inheritance
    Registry::instance()
        .register_class<ShapeCircle>("ShapeCircle")
        .inherits_from<Shape>("Shape")
        .property("radius", &ShapeCircle::radius, &ShapeCircle::setRadius)
        .property("color", &ShapeCircle::color, &ShapeCircle::setColor)
        .method("area", &ShapeCircle::area);

    // Register ShapeRectangle with inheritance
    Registry::instance()
        .register_class<ShapeRectangle>("ShapeRectangle")
        .inherits_from<Shape>("Shape")
        .property("width", &ShapeRectangle::width, &ShapeRectangle::setWidth)
        .property("height", &ShapeRectangle::height, &ShapeRectangle::setHeight)
        .property("color", &ShapeRectangle::color, &ShapeRectangle::setColor)
        .method("area", &ShapeRectangle::area)
        .method("setSize", &ShapeRectangle::setSize);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Register Rosetta metadata
    registerRosettaTypes();

    // Register QML types
    qmlRegisterType<rosetta::qt::QmlBridge>("Rosetta", 1, 0, "QmlBridge");
    qmlRegisterType<ShapeCircle>("Shapes", 1, 0, "ShapeCircle");
    qmlRegisterType<ShapeRectangle>("Shapes", 1, 0, "ShapeRectangle");

    QQmlApplicationEngine engine;

    // Create example objects
    ShapeCircle circle;
    circle.setX(150);
    circle.setY(150);
    circle.setRadius(60);

    ShapeRectangle rect;
    rect.setX(350);
    rect.setY(120);
    rect.setWidth(120);
    rect.setHeight(80);

    // Expose to QML
    engine.rootContext()->setContextProperty("exampleCircle", &circle);
    engine.rootContext()->setContextProperty("exampleRect", &rect);

    engine.loadFromModule("RosettaQmlDemo", "Main");

    return app.exec();
}

#include "main.moc"