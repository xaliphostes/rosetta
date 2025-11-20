#include <rosetta/extensions/generators/py_generator.h>
#include "../registration.h"

BEGIN_PY_MODULE(complex, "Python bindings for C++ classes using Rosetta introspection") {
    register_rosetta_classes();

    // *** IMPORTANT: Register function converters BEFORE binding classes ***
    // Register the std::function<Point(const Point&)> type converter
    // This allows Python lambdas to be converted to C++ std::function
    rosetta::py::register_function_converter<Point, const Point &>();

    BIND_PY_CLASS(Point);
    BIND_PY_CLASS(Triangle);
    BIND_PY_CLASS(Surface);
    BIND_PY_CLASS(Model);
}
END_PY_MODULE()