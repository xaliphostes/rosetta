#include <rosetta/extensions/qt/qt_property_editor.h>
#include <rosetta/rosetta.h>

// Modern nodeeditor 3.x API
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include <memory>

using namespace QtNodes;

// Forward declarations
template <typename T> class AlgorithmNodeDelegate;

// ============================================================================
// Data Type
// ============================================================================

class NumberData : public NodeData {
public:
    NumberData() : value_(0.0) {}
    explicit NumberData(double value) : value_(value) {}

    NodeDataType type() const override { return NodeDataType{"number", "Number"}; }

    double value() const { return value_; }

private:
    double value_;
};

// ============================================================================
// Algorithms
// ============================================================================

struct AddAlgorithm {
    double input_a = 0.0, input_b = 0.0, output = 0.0;
    void   compute() { output = input_a + input_b; }
};

struct MultiplyAlgorithm {
    double input_a = 1.0, input_b = 1.0, output = 1.0;
    void   compute() { output = input_a * input_b; }
};

struct ConstantAlgorithm {
    double value = 1.0, output = 1.0;
    void   compute() { output = value; }
};

// ============================================================================
// Register
// ============================================================================

void register_algorithms() {
    ROSETTA_REGISTER_CLASS(AddAlgorithm)
        .field("input_a", &AddAlgorithm::input_a)
        .field("input_b", &AddAlgorithm::input_b)
        .field("output", &AddAlgorithm::output)
        .method("compute", &AddAlgorithm::compute);

    ROSETTA_REGISTER_CLASS(MultiplyAlgorithm)
        .field("input_a", &MultiplyAlgorithm::input_a)
        .field("input_b", &MultiplyAlgorithm::input_b)
        .field("output", &MultiplyAlgorithm::output)
        .method("compute", &MultiplyAlgorithm::compute);

    ROSETTA_REGISTER_CLASS(ConstantAlgorithm)
        .field("value", &ConstantAlgorithm::value)
        .field("output", &ConstantAlgorithm::output)
        .method("compute", &ConstantAlgorithm::compute);
}

// ============================================================================
// Node Model
// ============================================================================

template <typename AlgorithmType> class AlgorithmNodeDelegate : public NodeDelegateModel {
public:
    AlgorithmNodeDelegate() {
        widget_      = new QWidget();
        auto *layout = new QVBoxLayout(widget_);
        layout->setContentsMargins(5, 5, 5, 5);

        const auto &metadata = rosetta::core::Registry::instance().get<AlgorithmType>();
        auto       *title    = new QLabel(QString::fromStdString(metadata.name()));
        title->setStyleSheet("font-weight: bold;");
        layout->addWidget(title);

        property_editor_ = new rosetta::qt::PropertyEditor<AlgorithmType>(&algorithm_);
        property_editor_->setPropertyChangedCallback([this](const std::string &) { compute(); });
        layout->addWidget(property_editor_);

        auto *btn = new QPushButton("Execute");
        btn->setStyleSheet("background-color: #3498db; color: white; padding: 5px;");
        connect(btn, &QPushButton::clicked, [this]() { compute(); });
        layout->addWidget(btn);

        compute();
    }

    QString caption() const override {
        return QString::fromStdString(
            rosetta::core::Registry::instance().get<AlgorithmType>().name());
    }

    QString name() const override { return caption(); }

    unsigned int nPorts(PortType portType) const override {
        return static_cast<unsigned int>(portType == PortType::In ? getInputPorts().size()
                                                                  : getOutputPorts().size());
    }

    NodeDataType dataType(PortType, PortIndex) const override { return NumberData().type(); }

    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex const portIndex) override {
        if (auto numberData = std::dynamic_pointer_cast<NumberData>(nodeData)) {
            auto inputs = getInputPorts();
            if (portIndex < inputs.size()) {
                rosetta::core::Registry::instance().get<AlgorithmType>().set_field(
                    algorithm_, inputs[portIndex], rosetta::core::Any(numberData->value()));
                compute();
            }
        }
    }

    std::shared_ptr<NodeData> outData(PortIndex portIndex) override {
        return portIndex < output_data_.size() ? output_data_[portIndex] : nullptr;
    }

    QWidget *embeddedWidget() override { return widget_; }

    bool portCaptionVisible(PortType, PortIndex) const override { return true; }

    QString portCaption(PortType portType, PortIndex portIndex) const override {
        // Can't use reference because ternary returns a temporary
        auto ports = portType == PortType::In ? getInputPorts() : getOutputPorts();
        return portIndex < ports.size() ? QString::fromStdString(ports[portIndex]) : "";
    }

private:
    void compute() {
        property_editor_->refresh();
        const auto &metadata = rosetta::core::Registry::instance().get<AlgorithmType>();
        metadata.invoke_method(algorithm_, "compute", {});

        auto outputs = getOutputPorts();
        output_data_.resize(outputs.size());
        for (size_t i = 0; i < outputs.size(); ++i) {
            auto value = metadata.get_field(algorithm_, outputs[i]);
            if (value.get_type_index() == std::type_index(typeid(double))) {
                output_data_[i] = std::make_shared<NumberData>(value.template as<double>());
            }
        }
        property_editor_->refresh();
        Q_EMIT dataUpdated(0);
    }

    std::vector<std::string> getInputPorts() const {
        std::vector<std::string> inputs;
        for (const auto &field :
             rosetta::core::Registry::instance().get<AlgorithmType>().fields()) {
            if (field.find("input") != std::string::npos)
                inputs.push_back(field);
        }
        return inputs;
    }

    std::vector<std::string> getOutputPorts() const {
        std::vector<std::string> outputs;
        for (const auto &field :
             rosetta::core::Registry::instance().get<AlgorithmType>().fields()) {
            if (field.find("output") != std::string::npos)
                outputs.push_back(field);
        }
        return outputs;
    }

    AlgorithmType                               algorithm_;
    QWidget                                    *widget_;
    rosetta::qt::PropertyEditor<AlgorithmType> *property_editor_;
    std::vector<std::shared_ptr<NumberData>>    output_data_;
};

// ============================================================================
// Custom GraphicsView with Manual Context Menu
// ============================================================================

class CustomGraphicsView : public GraphicsView {
public:
    CustomGraphicsView(BasicGraphicsScene *scene) : GraphicsView(scene), scene_(scene) {}

    void setDataFlowModel(std::shared_ptr<DataFlowGraphModel> model) { model_ = model; }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
        if (!model_) {
            std::cout << "⚠️  No model set!" << std::endl;
            return;
        }

        auto registry = model_->dataModelRegistry();
        if (!registry) {
            std::cout << "⚠️  No registry found!" << std::endl;
            return;
        }

        // Create context menu
        QMenu menu;

        auto categories = registry->categories();
        for (const auto &category : categories) {
            auto *categoryMenu = menu.addMenu(category);
            auto  models       = registry->registeredModelsCategoryAssociation();

            for (auto it = models.begin(); it != models.end(); ++it) {
                if (it->second == category) {
                    QString modelName = it->first;

                    auto *action = categoryMenu->addAction(modelName);
                    connect(action, &QAction::triggered, [this, modelName, event]() {
                        // Map to scene coordinates
                        QPointF scenePos = mapToScene(event->pos());

                        // Create node at clicked position
                        auto nodeId = model_->addNode(modelName);
                        model_->setNodeData(nodeId, NodeRole::Position, scenePos);
                    });
                }
            }
        }

        menu.exec(event->globalPos());
    }

private:
    BasicGraphicsScene                 *scene_;
    std::shared_ptr<DataFlowGraphModel> model_;
};

// ============================================================================
// Type aliases
// ============================================================================

using AddNode      = AlgorithmNodeDelegate<AddAlgorithm>;
using MultiplyNode = AlgorithmNodeDelegate<MultiplyAlgorithm>;
using ConstantNode = AlgorithmNodeDelegate<ConstantAlgorithm>;

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    register_algorithms();

    // Create registry
    auto registry = std::make_shared<NodeDelegateModelRegistry>();
    registry->registerModel<AddNode>("Math");
    registry->registerModel<MultiplyNode>("Math");
    registry->registerModel<ConstantNode>("Sources");

    // Create model
    auto dataFlowModel = std::make_shared<DataFlowGraphModel>(registry);

    // Create scene
    auto *scene = new BasicGraphicsScene(*dataFlowModel);

    // Create CUSTOM view with manual context menu
    auto *view = new CustomGraphicsView(scene);
    view->setDataFlowModel(dataFlowModel); // Give view access to model

    view->setWindowTitle("Node Calculator - Manual Context Menu");
    view->resize(1200, 800);
    view->setStyleSheet("QGraphicsView { background-color: #2c3e50; }");

    view->show();

    std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout <<   "║  Node Calculator - RIGHT-CLICK to add nodes!              ║" << std::endl;
    std::cout <<   "╚═══════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\n📋 Right-click anywhere in the window to open the node menu" << std::endl;
    std::cout << "   You should see categories: Math, Sources" << std::endl;

    return app.exec();
}