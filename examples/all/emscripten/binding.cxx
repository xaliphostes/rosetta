#include "../registration.h"
#include <rosetta/extensions/generators/em_generator.h>

// NOTE: Don't know yet how to bind ctors
// Emscipten requires explicit constructor argument types (requires compile-time types!)

BEGIN_EM_MODULE(allem) {
    register_rosetta_classes();

    BIND_EM_UTILITIES();

    BIND_EM_CLASS(Point,
        std::tuple<>, 
        std::tuple<int, int, int>
    );

    BIND_EM_CLASS(Triangle, 
        std::tuple<>, 
        std::tuple<int, int, int>
    );

    BIND_EM_CLASS(Surface,
        std::tuple<>,
        std::tuple<const std::vector<double> &, const std::vector<int> &>);

    BIND_EM_CLASS(Model,
        std::tuple<>
    );

}
END_EM_MODULE()
