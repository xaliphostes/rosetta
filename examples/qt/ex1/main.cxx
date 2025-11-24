// ============================================================================
// Example usage of Qt6 Property Editor with Rosetta
// ============================================================================

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QStackedWidget>

// Include Rosetta core
#include <rosetta/rosetta.h>

// Include the Qt property editor
#include <rosetta/extensions/qt/qt_property_editor.h>

// ============================================================================
// Example Classes to demonstrate the property editor
// ============================================================================

/**
 * @brief Simple 2D point class
 */
class Point {
public:
    double x = 0.0;
    double y = 0.0;

    Point() = default;
    Point(double x_, double y_) : x(x_), y(y_) {}

    double magnitude() const { return std::sqrt(x * x + y * y); }

    void normalize() {
        double mag = magnitude();
        if (mag > 0) {
            x /= mag;
            y /= mag;
        }
    }

    void scale(double factor) {
        x *= factor;
        y *= factor;
    }

    void translate(double dx, double dy) {
        x += dx;
        y += dy;
    }
};

/**
 * @brief A person class with various property types
 */
class Person {
public:
    std::string name        = "John Doe";
    int         age         = 30;
    double      height      = 1.75;
    bool        is_employed = true;
    std::string email       = "john@example.com";

    Person() = default;
    Person(const std::string &name_, int age_) : name(name_), age(age_) {}

    std::string greet() const { return "Hello, my name is " + name; }

    void birthday() { age++; }

    void setInfo(const std::string &new_name, int new_age) {
        name = new_name;
        age  = new_age;
    }

    bool isAdult() const { return age >= 18; }
};

/**
 * @brief A configuration class with many settings
 */
class AppConfig {
public:
    // Display settings
    int    window_width  = 1280;
    int    window_height = 720;
    bool   fullscreen    = false;
    double ui_scale      = 1.0;

    // Audio settings
    int  master_volume = 80;
    int  music_volume  = 70;
    int  sfx_volume    = 90;
    bool mute_all      = false;

    // Graphics settings
    int    quality_level = 2; // 0=Low, 1=Medium, 2=High, 3=Ultra
    bool   vsync_enabled = true;
    int    max_fps       = 60;
    double render_scale  = 1.0;
    bool   antialiasing  = true;

    // Network settings
    std::string server_address = "localhost";
    int         server_port    = 8080;
    int         timeout_ms     = 5000;

    // Paths
    std::string data_path  = "/data";
    std::string cache_path = "/cache";
    std::string log_path   = "/logs";

    // Feature flags
    bool enable_debug     = false;
    bool enable_profiling = false;
    bool enable_telemetry = false;

    AppConfig() = default;

    void resetToDefaults() { *this = AppConfig(); }

    void applyPreset(int preset) {
        switch (preset) {
        case 0: // Low
            quality_level = 0;
            render_scale  = 0.75;
            antialiasing  = false;
            max_fps       = 30;
            break;
        case 1: // Medium
            quality_level = 1;
            render_scale  = 1.0;
            antialiasing  = false;
            max_fps       = 60;
            break;
        case 2: // High
            quality_level = 2;
            render_scale  = 1.0;
            antialiasing  = true;
            max_fps       = 60;
            break;
        case 3: // Ultra
            quality_level = 3;
            render_scale  = 1.5;
            antialiasing  = true;
            max_fps       = 144;
            break;
        }
    }
};

/**
 * @brief A shape with vectors for demonstration
 */
class Shape {
public:
    std::string         name = "Polygon";
    std::vector<double> vertices_x;
    std::vector<double> vertices_y;
    std::vector<int>    indices;
    bool                filled     = true;
    double              line_width = 1.0;

    Shape() {
        // Default triangle
        vertices_x = {0.0, 1.0, 0.5};
        vertices_y = {0.0, 0.0, 1.0};
        indices    = {0, 1, 2};
    }

    int vertexCount() const { return static_cast<int>(vertices_x.size()); }

    void clear() {
        vertices_x.clear();
        vertices_y.clear();
        indices.clear();
    }

    void addVertex(double x, double y) {
        vertices_x.push_back(x);
        vertices_y.push_back(y);
    }
};

// ============================================================================
// Rosetta Registration
// ============================================================================

void registerClasses() {
    using namespace rosetta::core;

    // Register Point
    Registry::instance()
        .register_class<Point>("Point")
        .constructor<>()
        .constructor<double, double>()
        .field("x", &Point::x)
        .field("y", &Point::y)
        .method("magnitude", &Point::magnitude)
        .method("normalize", &Point::normalize)
        .method("scale", &Point::scale)
        .method("translate", &Point::translate);

    // Register Person
    Registry::instance()
        .register_class<Person>("Person")
        .constructor<>()
        .constructor<const std::string &, int>()
        .field("name", &Person::name)
        .field("age", &Person::age)
        .field("height", &Person::height)
        .field("is_employed", &Person::is_employed)
        .field("email", &Person::email)
        .method("greet", &Person::greet)
        .method("birthday", &Person::birthday)
        .method("setInfo", &Person::setInfo)
        .method("isAdult", &Person::isAdult);

    // Register AppConfig
    Registry::instance()
        .register_class<AppConfig>("AppConfig")
        .constructor<>()
        // Display
        .field("window_width", &AppConfig::window_width)
        .field("window_height", &AppConfig::window_height)
        .field("fullscreen", &AppConfig::fullscreen)
        .field("ui_scale", &AppConfig::ui_scale)
        // Audio
        .field("master_volume", &AppConfig::master_volume)
        .field("music_volume", &AppConfig::music_volume)
        .field("sfx_volume", &AppConfig::sfx_volume)
        .field("mute_all", &AppConfig::mute_all)
        // Graphics
        .field("quality_level", &AppConfig::quality_level)
        .field("vsync_enabled", &AppConfig::vsync_enabled)
        .field("max_fps", &AppConfig::max_fps)
        .field("render_scale", &AppConfig::render_scale)
        .field("antialiasing", &AppConfig::antialiasing)
        // Network
        .field("server_address", &AppConfig::server_address)
        .field("server_port", &AppConfig::server_port)
        .field("timeout_ms", &AppConfig::timeout_ms)
        // Paths
        .field("data_path", &AppConfig::data_path)
        .field("cache_path", &AppConfig::cache_path)
        .field("log_path", &AppConfig::log_path)
        // Feature flags
        .field("enable_debug", &AppConfig::enable_debug)
        .field("enable_profiling", &AppConfig::enable_profiling)
        .field("enable_telemetry", &AppConfig::enable_telemetry)
        // Methods
        .method("resetToDefaults", &AppConfig::resetToDefaults)
        .method("applyPreset", &AppConfig::applyPreset);

    // Register Shape
    Registry::instance()
        .register_class<Shape>("Shape")
        .constructor<>()
        .field("name", &Shape::name)
        .field("vertices_x", &Shape::vertices_x)
        .field("vertices_y", &Shape::vertices_y)
        .field("indices", &Shape::indices)
        .field("filled", &Shape::filled)
        .field("line_width", &Shape::line_width)
        .method("vertexCount", &Shape::vertexCount)
        .method("clear", &Shape::clear)
        .method("addVertex", &Shape::addVertex);
}

// ============================================================================
// Main Window
// ============================================================================

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Rosetta Qt Property Editor Demo");
        resize(1200, 800);

        createMenus();
        createCentralWidget();
        createStatusBar();
    }

private:
    void createMenus() {
        auto *file_menu = menuBar()->addMenu("&File");
        file_menu->addAction("&Refresh", [this]() { refreshAll(); }, QKeySequence::Refresh);
        file_menu->addSeparator();
        file_menu->addAction("&Quit", qApp, &QApplication::quit, QKeySequence::Quit);

        auto *view_menu = menuBar()->addMenu("&View");
        view_menu->addAction("Point Editor", [this]() { showPointEditor(); });
        view_menu->addAction("Person Editor", [this]() { showPersonEditor(); });
        view_menu->addAction("Config Editor", [this]() { showConfigEditor(); });
        view_menu->addAction("Shape Editor", [this]() { showShapeEditor(); });
        view_menu->addSeparator();
        view_menu->addAction("Multi-Person Editor", [this]() { showMultiPersonEditor(); });
    }

    void createCentralWidget() {
        auto *splitter = new QSplitter(Qt::Horizontal);

        // Left panel: Object list
        auto *list_widget = new QListWidget();
        list_widget->addItem("Point");
        list_widget->addItem("Person");
        list_widget->addItem("AppConfig");
        list_widget->addItem("Shape");
        list_widget->setMaximumWidth(150);

        connect(list_widget, &QListWidget::currentTextChanged, [this](const QString &text) {
            if (text == "Point")
                showPointEditor();
            else if (text == "Person")
                showPersonEditor();
            else if (text == "AppConfig")
                showConfigEditor();
            else if (text == "Shape")
                showShapeEditor();
        });

        splitter->addWidget(list_widget);

        // Right panel: Editor container
        editor_container_ = new QStackedWidget();
        splitter->addWidget(editor_container_);

        // Create editors
        point_inspector_  = new rosetta::qt::ObjectInspector<Point>(&point_);
        person_inspector_ = new rosetta::qt::ObjectInspector<Person>(&person_);
        config_inspector_ = new rosetta::qt::ObjectInspector<AppConfig>(&config_);
        shape_inspector_  = new rosetta::qt::ObjectInspector<Shape>(&shape_);

        // Set up change callbacks
        point_inspector_->propertyEditor()->setPropertyChangedCallback(
            [this](const std::string &field) {
                statusBar()->showMessage(
                    QString("Point.%1 changed").arg(QString::fromStdString(field)), 2000);
            });

        person_inspector_->propertyEditor()->setPropertyChangedCallback(
            [this](const std::string &field) {
                statusBar()->showMessage(
                    QString("Person.%1 changed").arg(QString::fromStdString(field)), 2000);
            });

        config_inspector_->propertyEditor()->setPropertyChangedCallback(
            [this](const std::string &field) {
                statusBar()->showMessage(
                    QString("AppConfig.%1 changed").arg(QString::fromStdString(field)), 2000);
            });

        // Group config fields
        config_inspector_->propertyEditor()->setFieldGroup("window_width", "Display");
        config_inspector_->propertyEditor()->setFieldGroup("window_height", "Display");
        config_inspector_->propertyEditor()->setFieldGroup("fullscreen", "Display");
        config_inspector_->propertyEditor()->setFieldGroup("ui_scale", "Display");

        config_inspector_->propertyEditor()->setFieldGroup("master_volume", "Audio");
        config_inspector_->propertyEditor()->setFieldGroup("music_volume", "Audio");
        config_inspector_->propertyEditor()->setFieldGroup("sfx_volume", "Audio");
        config_inspector_->propertyEditor()->setFieldGroup("mute_all", "Audio");

        config_inspector_->propertyEditor()->setFieldGroup("quality_level", "Graphics");
        config_inspector_->propertyEditor()->setFieldGroup("vsync_enabled", "Graphics");
        config_inspector_->propertyEditor()->setFieldGroup("max_fps", "Graphics");
        config_inspector_->propertyEditor()->setFieldGroup("render_scale", "Graphics");
        config_inspector_->propertyEditor()->setFieldGroup("antialiasing", "Graphics");

        editor_container_->addWidget(point_inspector_);
        editor_container_->addWidget(person_inspector_);
        editor_container_->addWidget(config_inspector_);
        editor_container_->addWidget(shape_inspector_);

        // Multi-person editor
        multi_person_editor_ = new rosetta::qt::MultiObjectPropertyEditor<Person>();
        multi_person_editor_->setObjects({&person_, &person2_, &person3_});
        editor_container_->addWidget(multi_person_editor_);

        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);

        setCentralWidget(splitter);

        // Show first editor
        list_widget->setCurrentRow(0);
    }

    void createStatusBar() { statusBar()->showMessage("Ready"); }

    void showPointEditor() { editor_container_->setCurrentWidget(point_inspector_); }

    void showPersonEditor() { editor_container_->setCurrentWidget(person_inspector_); }

    void showConfigEditor() { editor_container_->setCurrentWidget(config_inspector_); }

    void showShapeEditor() { editor_container_->setCurrentWidget(shape_inspector_); }

    void showMultiPersonEditor() { editor_container_->setCurrentWidget(multi_person_editor_); }

    void refreshAll() {
        point_inspector_->refresh();
        person_inspector_->refresh();
        config_inspector_->refresh();
        shape_inspector_->refresh();
        multi_person_editor_->refresh();
        statusBar()->showMessage("Refreshed all editors", 2000);
    }

    // Data objects
    Point     point_{3.0, 4.0};
    Person    person_{"Alice", 28};
    Person    person2_{"Bob", 35};
    Person    person3_{"Charlie", 42};
    AppConfig config_;
    Shape     shape_;

    // UI
    QStackedWidget                                 *editor_container_    = nullptr;
    rosetta::qt::ObjectInspector<Point>            *point_inspector_     = nullptr;
    rosetta::qt::ObjectInspector<Person>           *person_inspector_    = nullptr;
    rosetta::qt::ObjectInspector<AppConfig>        *config_inspector_    = nullptr;
    rosetta::qt::ObjectInspector<Shape>            *shape_inspector_     = nullptr;
    rosetta::qt::MultiObjectPropertyEditor<Person> *multi_person_editor_ = nullptr;
};

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[]) {
    // Register classes with Rosetta
    registerClasses();

    // Create Qt application
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}