#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>

// Example class demonstrating various container types
class DataContainer {
    std::vector<double>        values_;
    std::map<std::string, int> name_to_id_;
    std::set<std::string>      tags_;
    std::array<double, 3>      position_;

public:
    DataContainer() : position_{0.0, 0.0, 0.0} {}

    // Vector accessors
    const std::vector<double> &getValues() const { return values_; }
    void                       setValues(const std::vector<double> &v) { values_ = v; }

    // Map accessors
    const std::map<std::string, int> &getNameToId() const { return name_to_id_; }
    void setNameToId(const std::map<std::string, int> &m) { name_to_id_ = m; }

    // Set accessors
    const std::set<std::string> &getTags() const { return tags_; }
    void                         setTags(const std::set<std::string> &s) { tags_ = s; }

    // Array accessors
    const std::array<double, 3> &getPosition() const { return position_; }
    void                         setPosition(const std::array<double, 3> &p) { position_ = p; }

    // Utility methods
    void addValue(double v) { values_.push_back(v); }

    void addMapping(const std::string &name, int id) { name_to_id_[name] = id; }

    void addTag(const std::string &tag) { tags_.insert(tag); }

    bool hasTag(const std::string &tag) const { return tags_.find(tag) != tags_.end(); }

    int getId(const std::string &name) const {
        auto it = name_to_id_.find(name);
        return it != name_to_id_.end() ? it->second : -1;
    }

    size_t getValueCount() const { return values_.size(); }
    size_t getMappingCount() const { return name_to_id_.size(); }
    size_t getTagCount() const { return tags_.size(); }
};

// Another example with different container types
class MathUtils {
public:
    // Returns a fixed-size array
    std::array<int, 4> getFibonacci4() const { return {1, 1, 2, 3}; }

    // Returns a set of primes
    std::set<int> getPrimesUpTo20() const { return {2, 3, 5, 7, 11, 13, 17, 19}; }

    // Returns a frequency map
    std::map<int, int> getFrequencies(const std::vector<int> &numbers) const {
        std::map<int, int> freq;
        for (int n : numbers) {
            freq[n]++;
        }
        return freq;
    }

    // Transforms a map
    std::map<std::string, double> scaleMap(const std::map<std::string, double> &input,
                                           double                               scale) const {
        std::map<std::string, double> result;
        for (const auto &[key, value] : input) {
            result[key] = value * scale;
        }
        return result;
    }

    // Set operations
    std::set<int> setIntersection(const std::set<int> &a, const std::set<int> &b) const {
        std::set<int> result;
        std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                              std::inserter(result, result.begin()));
        return result;
    }

    std::set<int> setUnion(const std::set<int> &a, const std::set<int> &b) const {
        std::set<int> result;
        std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                       std::inserter(result, result.begin()));
        return result;
    }

    // Array operations
    std::array<double, 3> normalizeVector(const std::array<double, 3> &v) const {
        double magnitude = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (magnitude < 1e-10)
            return {0.0, 0.0, 0.0};
        return {v[0] / magnitude, v[1] / magnitude, v[2] / magnitude};
    }

    double dotProduct(const std::array<double, 3> &a, const std::array<double, 3> &b) const {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }
};

// ------------------------------------------------------
// Registration with Rosetta
// ------------------------------------------------------
void register_container_classes() {
    ROSETTA_REGISTER_CLASS(DataContainer)
        .constructor<>()
        .property("values", &DataContainer::getValues, &DataContainer::setValues)
        .property("name_to_id", &DataContainer::getNameToId, &DataContainer::setNameToId)
        .property("tags", &DataContainer::getTags, &DataContainer::setTags)
        .property("position", &DataContainer::getPosition, &DataContainer::setPosition)
        .method("addValue", &DataContainer::addValue)
        .method("addMapping", &DataContainer::addMapping)
        .method("addTag", &DataContainer::addTag)
        .method("hasTag", &DataContainer::hasTag)
        .method("getId", &DataContainer::getId)
        .method("getValueCount", &DataContainer::getValueCount)
        .method("getMappingCount", &DataContainer::getMappingCount)
        .method("getTagCount", &DataContainer::getTagCount);

    ROSETTA_REGISTER_CLASS(MathUtils)
        .constructor<>()
        .method("getFibonacci4", &MathUtils::getFibonacci4)
        .method("getPrimesUpTo20", &MathUtils::getPrimesUpTo20)
        .method("getFrequencies", &MathUtils::getFrequencies)
        .method("scaleMap", &MathUtils::scaleMap)
        .method("setIntersection", &MathUtils::setIntersection)
        .method("setUnion", &MathUtils::setUnion)
        .method("normalizeVector", &MathUtils::normalizeVector)
        .method("dotProduct", &MathUtils::dotProduct);
}

// ------------------------------------------------------
// Python Module Definition
// ------------------------------------------------------
BEGIN_PY_MODULE(rosetta_example, "Python bindings demonstrating map, set, array support") {
    register_container_classes();
    BIND_PY_CLASS(DataContainer);
    BIND_PY_CLASS(MathUtils);
}
END_PY_MODULE()