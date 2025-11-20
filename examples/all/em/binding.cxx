#include "../registration.h"
#include <rosetta/extensions/generators/em_generator.h>

BEGIN_EM_MODULE(shapes) {
    register_rosetta_classes();

    BIND_EM_UTILITIES();

    // Bind abstract base class
    BIND_EM_CLASS(Vectot3D, std::tuple<> std::tuple<int, int, int>);

    // Bind derived shape classes
    BIND_EM_CLASS(Triangle, std::tuple<>, std::tuple<int, int, int>);

    BIND_EM_CLASS(
        Surface, std::tuple<>,
        std::tuple<const std::vector<double> & positions, const std::vector<int> & indices>);

    // Square derives from Rectangle2D (two levels of inheritance)
    BIND_EM_CLASS(Model, std::tuple<>);

}
END_EM_MODULE()
