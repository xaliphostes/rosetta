#include "../registration.h"
#include <rosetta/extensions/generators/js_generator.h>

BEGIN_JS_MODULE(alljs) {
    register_rosetta_classes();

    BIND_JS_VECTOR(Point);
    BIND_JS_VECTOR(Triangle);

    // Register function types needed by Surface::transform
    BIND_JS_FUNCTION_TYPE(Point, const Point&);

    BIND_JS_CLASS(Triangle);
    BIND_JS_CLASS(Surface);
    BIND_JS_CLASS(Model);
}
END_JS_MODULE()
