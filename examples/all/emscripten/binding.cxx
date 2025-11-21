#include "../registration.h"
#include <rosetta/extensions/generators/em_generator.h>

// NOTE: Don't know yet how to bind ctors
// Emscipten requires explicit constructor argument types (requires compile-time types!)

Surface *buildSurface(const em::val &vertices, const em::val &indices) {
    std::vector<double> verts = rosetta::em_gen::EmValHelper::toVector<double>(vertices);
    std::vector<int>    idxs  = rosetta::em_gen::EmValHelper::toVector<int>(indices);
    return new Surface(verts, idxs);
}

BEGIN_EM_MODULE(allem) {
    register_rosetta_classes();

    // 1. Bind basic classes
    BIND_EM_CLASS_AUTO(Point, std::tuple<int, int, int>);
    BIND_EM_CLASS_AUTO(Triangle, std::tuple<int, int, int>);
    BIND_EM_CLASS_AUTO(Model, std::tuple<>);

    // 2. Register vector types
    BIND_EM_VECTOR_TYPE(Point);
    BIND_EM_VECTOR_TYPE(Triangle);
    BIND_EM_VECTOR_TYPE(Surface);

    // 3. Register function callback types
    BIND_EM_FUNCTION_TYPE(Point, const Point&);

    // 4. Bind classes that use callbacks
    BIND_EM_CLASS_FACTORY(Surface, &buildSurface);
}
END_EM_MODULE()
