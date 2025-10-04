#include "../../classes_demo.h"
#include <rosetta/generators/py.h>

// pybind11 module definition using automatic binding
PYBIND11_MODULE(rosetta, m) {
    m.doc() = "Automatic Python bindings using C++ introspection";

    rosetta::PyGenerator generator(m);
    generator.bind_classes<Person, Vehicle>();
}

