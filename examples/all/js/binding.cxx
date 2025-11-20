#include <rosetta/extensions/generators/js_generator.h>
#include "../registration.h"

BEGIN_JS_MODULE(mymodule) {
    register_rosetta_classes();
    generator.bind_class<Point>().bind_class<Triangle>().bind_class<Surface>().bind_class<Model>().add_utilities();
}
END_JS_MODULE()
