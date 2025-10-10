// examples/python/functions/binding.cxx
#include <cmath>
#include <rosetta/function_registry.h>
#include <rosetta/generators/py.h>

// Define standalone C++ functions
double calculateDistance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

std::string greet(const std::string &name) {
    return "Hello, " + name;
}

int add(int a, int b) {
    return a + b;
}

// More complex function with vector
std::vector<double> normalize(const std::vector<double> &values) {
    double sum = 0.0;
    for (double v : values) {
        sum += v;
    }

    std::vector<double> result;
    for (double v : values) {
        result.push_back(v / sum);
    }
    return result;
}

// Register all functions
REGISTER_FUNCTION(calculateDistance);
REGISTER_FUNCTION(greet);
REGISTER_FUNCTION(add);
REGISTER_FUNCTION(normalize);

// Python module binding
PYBIND11_MODULE(rosetta_functions, m) {
    m.doc() = "Python bindings for standalone C++ functions";

    rosetta::PyGenerator generator(m);

    // Bind all registered functions automatically!
    rosetta::bindAllFunctions(generator);
}