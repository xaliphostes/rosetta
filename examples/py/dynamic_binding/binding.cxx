#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>

// Custom classes to use in containers
struct Color {
    int r, g, b, a;
    Color() : r(0), g(0), b(0), a(255) {}
    Color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}
};

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
};

// Class using various container types with custom types
class Scene {
    std::vector<Vec3>                    positions_;
    std::map<std::string, Color>         materials_;
    std::set<int>                        visible_ids_;
    std::array<double, 16>               transform_matrix_;
    std::unordered_map<int, std::string> id_to_name_;
    std::unordered_set<std::string>      active_layers_;
    std::deque<Vec3>                     path_;

public:
    Scene() {
        // Initialize transform matrix to identity
        for (size_t i = 0; i < 16; ++i) {
            transform_matrix_[i] = (i % 5 == 0) ? 1.0 : 0.0;
        }
    }

    // Getters and setters for all container types
    const std::vector<Vec3> &getPositions() const { return positions_; }
    void                     setPositions(const std::vector<Vec3> &p) { positions_ = p; }

    const std::map<std::string, Color> &getMaterials() const { return materials_; }
    void setMaterials(const std::map<std::string, Color> &m) { materials_ = m; }

    const std::set<int> &getVisibleIds() const { return visible_ids_; }
    void                 setVisibleIds(const std::set<int> &ids) { visible_ids_ = ids; }

    const std::array<double, 16> &getTransformMatrix() const { return transform_matrix_; }
    void setTransformMatrix(const std::array<double, 16> &m) { transform_matrix_ = m; }

    const std::unordered_map<int, std::string> &getIdToName() const { return id_to_name_; }
    void setIdToName(const std::unordered_map<int, std::string> &m) { id_to_name_ = m; }

    const std::unordered_set<std::string> &getActiveLayers() const { return active_layers_; }
    void setActiveLayers(const std::unordered_set<std::string> &layers) { active_layers_ = layers; }

    const std::deque<Vec3> &getPath() const { return path_; }
    void                    setPath(const std::deque<Vec3> &p) { path_ = p; }

    // Utility methods
    void addPosition(const Vec3 &pos) { positions_.push_back(pos); }

    void addMaterial(const std::string &name, const Color &color) { materials_[name] = color; }

    void addVisibleId(int id) { visible_ids_.insert(id); }

    void addToPath(const Vec3 &pos) { path_.push_back(pos); }

    void registerObject(int id, const std::string &name) { id_to_name_[id] = name; }

    void activateLayer(const std::string &layer) { active_layers_.insert(layer); }

    size_t getPositionCount() const { return positions_.size(); }
    size_t getMaterialCount() const { return materials_.size(); }
    size_t getVisibleIdCount() const { return visible_ids_.size(); }
    size_t getPathLength() const { return path_.size(); }
};

// Registration
void register_dynamic_binding_classes() {
    // Register basic custom classes
    ROSETTA_REGISTER_CLASS(Vec3)
        .constructor<>()
        .constructor<double, double, double>()
        .field("x", &Vec3::x)
        .field("y", &Vec3::y)
        .field("z", &Vec3::z);

    ROSETTA_REGISTER_CLASS(Color)
        .constructor<>()
        // .constructor<int, int, int>()
        .constructor<int, int, int, int>()
        .field("r", &Color::r)
        .field("g", &Color::g)
        .field("b", &Color::b)
        .field("a", &Color::a);

    ROSETTA_REGISTER_CLASS(Scene)
        .constructor<>()
        .property("positions", &Scene::getPositions, &Scene::setPositions)
        .property("materials", &Scene::getMaterials, &Scene::setMaterials)
        .property("visible_ids", &Scene::getVisibleIds, &Scene::setVisibleIds)
        .property("transform_matrix", &Scene::getTransformMatrix, &Scene::setTransformMatrix)
        .property("id_to_name", &Scene::getIdToName, &Scene::setIdToName)
        .property("active_layers", &Scene::getActiveLayers, &Scene::setActiveLayers)
        .property("path", &Scene::getPath, &Scene::setPath)
        .method("addPosition", &Scene::addPosition)
        .method("addMaterial", &Scene::addMaterial)
        .method("addVisibleId", &Scene::addVisibleId)
        .method("addToPath", &Scene::addToPath)
        .method("registerObject", &Scene::registerObject)
        .method("activateLayer", &Scene::activateLayer)
        .method("getPositionCount", &Scene::getPositionCount)
        .method("getMaterialCount", &Scene::getMaterialCount)
        .method("getVisibleIdCount", &Scene::getVisibleIdCount)
        .method("getPathLength", &Scene::getPathLength);
}

// Python Module Definition
BEGIN_PY_MODULE(rosetta_example, "Demonstration of dynamic container type binding") {
    register_dynamic_binding_classes();

    // *** CRITICAL: Register container types BEFORE binding classes ***
    // This allows the automatic conversion system to work properly

    rosetta::py::bind_vector_type<Vec3>();
    rosetta::py::bind_vector_type<Color>();

    rosetta::py::bind_map_type<std::string, Color>();

    rosetta::py::bind_array_type<double, 16>(); // 4x4 transformation matrix

    rosetta::py::bind_unordered_map_type<int, std::string>();
    rosetta::py::bind_unordered_map_type<std::string, int>();

    rosetta::py::bind_unordered_set_type<std::string>();
    rosetta::py::bind_unordered_set_type<int>();

    rosetta::py::bind_deque_type<Vec3>();

    // Now bind the classes
    BIND_PY_CLASS(Vec3);
    BIND_PY_CLASS(Color);
    BIND_PY_CLASS(Scene);
}
END_PY_MODULE()