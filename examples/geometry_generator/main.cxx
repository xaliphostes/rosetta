// ============================================================================
// Custom Binding Generator for Geometry Project
// 
// This generator links against the geometry project and calls the Rosetta
// registration function BEFORE generating bindings, so the generator can
// discover all registered classes.
// ============================================================================

#include <rosetta/extensions/generators/BindingGeneratorLib.h>

// Include your project's registration header
// This brings in register_rosetta_classes() and all the geometry types
#include "registration.h"

int main(int argc, char* argv[]) {
    // IMPORTANT: Call registration BEFORE running the generator
    // This populates rosetta::Registry::instance() with class metadata
    register_rosetta_classes();
    
    // Now run the binding generator
    // It will query the registry and find Point, Triangle, Surface, Model
    return BindingGeneratorLib::run(argc, argv);
}
