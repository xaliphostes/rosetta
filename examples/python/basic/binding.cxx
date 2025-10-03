#include <rosetta/generators/py.h>
#include "../../classes_demo.h"

// pybind11 module definition using automatic binding
PYBIND11_MODULE(rosettapy, m)
{
    m.doc() = "Automatic Python bindings using C++ introspection";

    rosetta::PyGenerator generator(m);
    generator.bind_classes<Person, Vehicle>();
}
