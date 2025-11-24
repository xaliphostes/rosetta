// ============================================================================
// Qt6 QML 3D Scene Editor - Main Entry Point
// ============================================================================

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSurfaceFormat>

#include "rosetta_registration.h"
#include "scene_objects.h"

int main(int argc, char *argv[]) {
    // Enable high-DPI scaling
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);

    // Set application info
    app.setApplicationName("Rosetta 3D Editor");
    app.setOrganizationName("Rosetta");
    app.setApplicationVersion("1.0.0");

    // Set up 3D surface format
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4); // MSAA
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Set Qt Quick style
    QQuickStyle::setStyle("Fusion");

    // Register QML types
    rosetta::qt3d::registerQmlTypes();

    // Register Rosetta classes
    rosetta::qt3d::registerRosettaClasses();

    // Create QML engine
    QQmlApplicationEngine engine;

    // Handle QML loading errors
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    // Load main QML file
    // When using qt_add_qml_module, the path is: qrc:/qt/qml/<URI>/Main.qml
    engine.loadFromModule("Rosetta3DEditor", "Main");

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }

    return app.exec();
}