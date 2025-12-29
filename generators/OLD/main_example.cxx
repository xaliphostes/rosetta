// ============================================================================
// Example: User's Custom Binding Generator
// 
// Copy this file to your project and modify it:
// 1. Include your project's registration header
// 2. Call your registration function before running the generator
// 3. Build this with your project linked
// ============================================================================

#include "BindingGeneratorLib.h"

// TODO: Include your project's registration header
// #include "myproject/bindings/registration.h"

int main(int argc, char* argv[]) {
    // TODO: Call your Rosetta registration function
    // This populates the Rosetta registry with your classes' metadata
    // myproject_rosetta::register_classes();
    
    // Example for the "arch" project:
    // arch_rosetta::register_arch3_classes();
    
    // Run the binding generator
    // It will query rosetta::Registry::instance() to get class metadata
    return BindingGeneratorLib::run(argc, argv);
}

// ============================================================================
// Build instructions:
//
// Your CMakeLists.txt should:
// 1. Add the binding generator source files
// 2. Link against your library (which provides the registration function)
// 3. Link against Rosetta
//
// Example:
//   add_executable(my_binding_generator
//       binding_generator/main_custom.cxx
//   )
//   target_include_directories(my_binding_generator PRIVATE
//       binding_generator/
//       ${ROSETTA_INCLUDE_DIR}
//   )
//   target_link_libraries(my_binding_generator PRIVATE
//       my_library          # Your library with Rosetta registration
//       nlohmann_json::nlohmann_json
//   )
// ============================================================================
