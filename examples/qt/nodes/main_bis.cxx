#include <rosetta/rosetta.h>
#include <rosetta/extensions/qt/qt_property_editor.h>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/GraphicsView>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/NodeData>

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QContextMenuEvent>
#include <QGroupBox>
#include <QScrollArea>
#include <iostream>
#include <memory>

using namespace QtNodes;

// ============================================================================
// Generic NodeData Wrapper - Wraps ANY Type
// ============================================================================

template<typename T>
class GenericData : public NodeData {
public:
    GenericData() = default;
    explicit GenericData(const T& value) : value_(value) {}
    
    NodeDataType type() const override {
        // Try to get type name from Rosetta, fall back to typeid
        try {
            const auto& metadata = rosetta::core::Registry::instance().get<T>();
            return NodeDataType{metadata.name().c_str(), metadata.name().c_str()};
        } catch (...) {
            // Type not registered, use typeid name
            return NodeDataType{typeid(T).name(), typeid(T).name()};
        }
    }
    
    const T& value() const { return value_; }
    T& value() { return value_; }
    void setValue(const T& v) { value_ = v; }
    
private:
    T value_;
};

// Specialization for basic types
class DoubleData : public NodeData {
public:
    DoubleData() : value_(0.0) {}
    explicit DoubleData(double v) : value_(v) {}
    NodeDataType type() const override { return NodeDataType{"double", "double"}; }
    double value() const { return value_; }
    void setValue(double v) { value_ = v; }
private:
    double value_;
};

// ============================================================================
// Example Domain Classes
// ============================================================================

struct Point3D {
    double x = 0.0, y = 0.0, z = 0.0;
    
    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
    
    void scale(double factor) {
        x *= factor;
        y *= factor;
        z *= factor;
    }
};

struct BoundingBox {
    Point3D min;
    Point3D max;
    
    Point3D center() const {
        return Point3D{
            (min.x + max.x) / 2.0,
            (min.y + max.y) / 2.0,
            (min.z + max.z) / 2.0
        };
    }
    
    Point3D size() const {
        return Point3D{
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        };
    }
    
    void expand(double amount) {
        min.x -= amount; min.y -= amount; min.z -= amount;
        max.x += amount; max.y += amount; max.z += amount;
    }
    
    bool contains(const Point3D& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }
};

// ============================================================================
// Register Domain Classes with Rosetta
// ============================================================================

void register_types() {
    ROSETTA_REGISTER_CLASS(Point3D)
        .field("x", &Point3D::x)
        .field("y", &Point3D::y)
        .field("z", &Point3D::z)
        .method("length", &Point3D::length)
        .method("normalize", &Point3D::normalize)
        .method("scale", &Point3D::scale);
    
    ROSETTA_REGISTER_CLASS(BoundingBox)
        .field("min", &BoundingBox::min)
        .field("max", &BoundingBox::max)
        .method("center", &BoundingBox::center)
        .method("size", &BoundingBox::size)
        .method("expand", &BoundingBox::expand)
        .method("contains", &BoundingBox::contains);
}

// ============================================================================
// FULLY GENERIC Node Delegate - Exposes ALL Fields and Methods
// ============================================================================

template<typename T>
class FullyGenericNodeDelegate : public NodeDelegateModel {
public:
    FullyGenericNodeDelegate() {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        
        // Create main widget
        auto* mainWidget = new QWidget();
        auto* mainLayout = new QVBoxLayout(mainWidget);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);
        
        // Title
        auto* titleLabel = new QLabel(QString::fromStdString(metadata.name()));
        titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #2c3e50;");
        mainLayout->addWidget(titleLabel);
        
        // === FIELDS SECTION ===
        auto* fieldsGroup = new QGroupBox("Fields");
        fieldsGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
        auto* fieldsLayout = new QVBoxLayout(fieldsGroup);
        
        property_editor_ = new rosetta::qt::PropertyEditor<T>(&object_);
        property_editor_->setPropertyChangedCallback([this](const std::string& field) {
            std::cout << "📝 Field '" << field << "' changed in " 
                      << rosetta::core::Registry::instance().get<T>().name() << std::endl;
            // Emit data update for all output ports (all fields can be outputs)
            for (unsigned int i = 0; i < nPorts(PortType::Out); ++i) {
                Q_EMIT dataUpdated(i);
            }
        });
        fieldsLayout->addWidget(property_editor_);
        mainLayout->addWidget(fieldsGroup);
        
        // === METHODS SECTION ===
        auto* methodsGroup = new QGroupBox("Methods");
        methodsGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
        auto* methodsLayout = new QVBoxLayout(methodsGroup);
        
        // Create a button for each method
        for (const auto& methodName : metadata.methods()) {
            auto* methodBtn = new QPushButton(QString::fromStdString(methodName));
            methodBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #3498db;
                    color: white;
                    padding: 6px 12px;
                    border-radius: 3px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #2980b9;
                }
                QPushButton:pressed {
                    background-color: #21618c;
                }
            )");
            
            connect(methodBtn, &QPushButton::clicked, [this, methodName]() {
                invokeMethod(methodName);
            });
            
            methodsLayout->addWidget(methodBtn);
        }
        mainLayout->addWidget(methodsGroup);
        
        mainLayout->addStretch();
        
        // Wrap in scroll area for long content
        auto* scrollArea = new QScrollArea();
        scrollArea->setWidget(mainWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setMinimumWidth(250);
        scrollArea->setMaximumWidth(400);
        
        widget_ = scrollArea;
        
        // Initialize output data for all fields
        output_data_.resize(metadata.fields().size());
        updateAllOutputs();
    }
    
    QString caption() const override {
        return QString::fromStdString(rosetta::core::Registry::instance().get<T>().name());
    }
    
    QString name() const override { return caption(); }
    
    // ALL fields are exposed as BOTH input and output ports!
    unsigned int nPorts(PortType portType) const override {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        return static_cast<unsigned int>(metadata.fields().size());
    }
    
    NodeDataType dataType(PortType portType, PortIndex portIndex) const override {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        auto fields = metadata.fields();
        
        if (portIndex < fields.size()) {
            std::string fieldName = fields[portIndex];
            // rosetta::core::Any value = metadata.get_field(object_, fieldName);
            rosetta::core::Any value = metadata.get_field(const_cast<T&>(object_), fieldName);
            
            // Return type based on actual field type
            std::type_index typeIdx = value.get_type_index();
            
            if (typeIdx == std::type_index(typeid(double))) {
                return DoubleData().type();
            } else if (typeIdx == std::type_index(typeid(Point3D))) {
                return NodeDataType{"Point3D", "Point3D"};
            } else if (typeIdx == std::type_index(typeid(BoundingBox))) {
                return NodeDataType{"BoundingBox", "BoundingBox"};
            }
            
            // Generic fallback
            return NodeDataType{value.type_name().c_str(), value.type_name().c_str()};
        }
        
        return NodeDataType{"unknown", "Unknown"};
    }
    
    // Receive data on ANY field (all fields are inputs)
    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex const portIndex) override {
        if (!nodeData) return;
        
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        auto fields = metadata.fields();
        
        if (portIndex >= fields.size()) return;
        
        std::string fieldName = fields[portIndex];
        
        // Try different data types
        if (auto doubleData = std::dynamic_pointer_cast<DoubleData>(nodeData)) {
            metadata.set_field(object_, fieldName, rosetta::core::Any(doubleData->value()));
            std::cout << "  ← Received double " << doubleData->value() 
                      << " on field '" << fieldName << "'" << std::endl;
        }
        else if (auto point3dData = std::dynamic_pointer_cast<GenericData<Point3D>>(nodeData)) {
            metadata.set_field(object_, fieldName, rosetta::core::Any(point3dData->value()));
            std::cout << "  ← Received Point3D on field '" << fieldName << "'" << std::endl;
        }
        else if (auto bboxData = std::dynamic_pointer_cast<GenericData<BoundingBox>>(nodeData)) {
            metadata.set_field(object_, fieldName, rosetta::core::Any(bboxData->value()));
            std::cout << "  ← Received BoundingBox on field '" << fieldName << "'" << std::endl;
        }
        
        // Update property editor display
        property_editor_->refresh();
        
        // Update all outputs since object state changed
        updateAllOutputs();
    }
    
    // Send data from ANY field (all fields are outputs)
    std::shared_ptr<NodeData> outData(PortIndex portIndex) override {
        return portIndex < output_data_.size() ? output_data_[portIndex] : nullptr;
    }
    
    QWidget* embeddedWidget() override { return widget_; }
    
    bool portCaptionVisible(PortType, PortIndex) const override { return true; }
    
    QString portCaption(PortType portType, PortIndex portIndex) const override {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        auto fields = metadata.fields();
        
        if (portIndex < fields.size()) {
            QString caption = QString::fromStdString(fields[portIndex]);
            
            // Add indicator for port direction
            if (portType == PortType::In) {
                return "→ " + caption;  // Input indicator
            } else {
                return caption + " →";  // Output indicator
            }
        }
        
        return "";
    }
    
private:
    void invokeMethod(const std::string& methodName) {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        
        std::cout << "\n🔧 Invoking method: " << metadata.name() 
                  << "::" << methodName << "()" << std::endl;
        
        try {
            // Try to invoke with no arguments first
            rosetta::core::Any result = metadata.invoke_method(object_, methodName, {});
            
            std::cout << "  ✓ Method executed successfully" << std::endl;
            
            // If method returns a value, show it
            if (result.has_value()) {
                std::cout << "  → Return value: " << result.toString() << std::endl;
            }
            
            // Update display and outputs
            property_editor_->refresh();
            updateAllOutputs();
            
        } catch (const std::exception& e) {
            std::cout << "  ✗ Error: " << e.what() << std::endl;
        }
    }
    
    void updateAllOutputs() {
        const auto& metadata = rosetta::core::Registry::instance().get<T>();
        auto fields = metadata.fields();
        
        output_data_.clear();
        output_data_.resize(fields.size());
        
        for (size_t i = 0; i < fields.size(); ++i) {
            rosetta::core::Any value = metadata.get_field(object_, fields[i]);
            
            // Create appropriate NodeData based on type
            std::type_index typeIdx = value.get_type_index();
            
            if (typeIdx == std::type_index(typeid(double))) {
                output_data_[i] = std::make_shared<DoubleData>(value.template as<double>());
            } else if (typeIdx == std::type_index(typeid(Point3D))) {
                output_data_[i] = std::make_shared<GenericData<Point3D>>(value.template as<Point3D>());
            } else if (typeIdx == std::type_index(typeid(BoundingBox))) {
                output_data_[i] = std::make_shared<GenericData<BoundingBox>>(value.template as<BoundingBox>());
            }
        }
        
        // Notify all output ports
        for (unsigned int i = 0; i < nPorts(PortType::Out); ++i) {
            Q_EMIT dataUpdated(i);
        }
    }
    
    T object_;
    QWidget* widget_;
    rosetta::qt::PropertyEditor<T>* property_editor_;
    std::vector<std::shared_ptr<NodeData>> output_data_;
};

// ============================================================================
// Custom GraphicsView with Context Menu
// ============================================================================

class CustomGraphicsView : public GraphicsView {
public:
    CustomGraphicsView(BasicGraphicsScene* scene) : GraphicsView(scene), scene_(scene) {}
    void setDataFlowModel(std::shared_ptr<DataFlowGraphModel> model) { model_ = model; }
    
protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
        if (!model_) return;
        auto registry = model_->dataModelRegistry();
        if (!registry) return;
        
        QMenu menu;
        auto categories = registry->categories();
        
        for (const auto& category : categories) {
            auto* categoryMenu = menu.addMenu(category);
            auto models = registry->registeredModelsCategoryAssociation();
            
            for (auto it = models.begin(); it != models.end(); ++it) {
                if (it->second == category) {
                    QString modelName = it->first;
                    auto* action = categoryMenu->addAction(modelName);
                    connect(action, &QAction::triggered, [this, modelName, event]() {
                        QPointF scenePos = mapToScene(event->pos());
                        auto nodeId = model_->addNode(modelName);
                        model_->setNodeData(nodeId, NodeRole::Position, scenePos);
                    });
                }
            }
        }
        
        menu.exec(event->globalPos());
    }
    
private:
    BasicGraphicsScene* scene_;
    std::shared_ptr<DataFlowGraphModel> model_;
};

// ============================================================================
// Type Aliases
// ============================================================================

using Point3DNode = FullyGenericNodeDelegate<Point3D>;
using BoundingBoxNode = FullyGenericNodeDelegate<BoundingBox>;

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    std::cout << "🚀 Starting Fully Generic Node Editor..." << std::endl;
    std::cout << "   All fields exposed as both inputs AND outputs!" << std::endl;
    std::cout << "   All methods exposed as callable operations!" << std::endl;
    
    register_types();
    
    // Create registry
    auto registry = std::make_shared<NodeDelegateModelRegistry>();
    registry->registerModel<Point3DNode>("Geometry");
    registry->registerModel<BoundingBoxNode>("Geometry");
    
    std::cout << "\n✓ Registered fully generic nodes" << std::endl;
    
    // Create model, scene, and view
    auto dataFlowModel = std::make_shared<DataFlowGraphModel>(registry);
    auto* scene = new BasicGraphicsScene(*dataFlowModel);
    auto* view = new CustomGraphicsView(scene);
    view->setDataFlowModel(dataFlowModel);
    
    view->setWindowTitle("Fully Generic Node Editor - All Fields & Methods Exposed");
    view->resize(1400, 900);
    view->setStyleSheet("QGraphicsView { background-color: #2c3e50; }");
    view->show();
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Fully Generic Node Editor - Complete Introspection      ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\n📋 How It Works:" << std::endl;
    std::cout << "   • ALL fields are exposed as BOTH input AND output ports" << std::endl;
    std::cout << "   • ALL methods are exposed as clickable buttons" << std::endl;
    std::cout << "   • No naming conventions required (no 'input_' or 'output_')" << std::endl;
    std::cout << "   • Complete bidirectional data flow" << std::endl;
    
    std::cout << "\n🎯 Available Nodes:" << std::endl;
    std::cout << "   Point3D:" << std::endl;
    std::cout << "     Fields: x, y, z (all editable, all connectable)" << std::endl;
    std::cout << "     Methods: length(), normalize(), scale()" << std::endl;
    std::cout << "   BoundingBox:" << std::endl;
    std::cout << "     Fields: min, max (all editable, all connectable)" << std::endl;
    std::cout << "     Methods: center(), size(), expand(), contains()" << std::endl;
    
    std::cout << "\n💡 Example Workflow:" << std::endl;
    std::cout << "   1. Create two Point3D nodes" << std::endl;
    std::cout << "   2. Set values in first: x=1, y=2, z=3" << std::endl;
    std::cout << "   3. Connect first node's 'x' output to second node's 'x' input" << std::endl;
    std::cout << "   4. Connect first node's 'y' output to second node's 'y' input" << std::endl;
    std::cout << "   5. Connect first node's 'z' output to second node's 'z' input" << std::endl;
    std::cout << "   6. Click 'normalize()' on second node" << std::endl;
    std::cout << "   7. See normalized values automatically update!" << std::endl;
    
    std::cout << "\n🔥 Advanced Example:" << std::endl;
    std::cout << "   1. Create Point3D node for 'min' corner" << std::endl;
    std::cout << "   2. Create Point3D node for 'max' corner" << std::endl;
    std::cout << "   3. Create BoundingBox node" << std::endl;
    std::cout << "   4. Connect min Point3D's output → BoundingBox's 'min' input" << std::endl;
    std::cout << "   5. Connect max Point3D's output → BoundingBox's 'max' input" << std::endl;
    std::cout << "   6. Click 'center()' on BoundingBox to compute center" << std::endl;
    std::cout << "   7. Click 'expand()' to grow the bounding box" << std::endl;
    
    std::cout << "\n" << std::endl;
    
    return app.exec();
}