#pragma once
#include <string>
#include <vector>
#include <set>

// ============================================================================
// Project Configuration - Loaded from JSON/YAML config file
// ============================================================================

struct TargetConfig {
    bool enabled = false;
    std::string output_dir;  // Override default output directory
    std::vector<std::string> extra_sources;  // Additional source files
    std::vector<std::string> extra_libs;     // Additional libraries to link
};

struct ProjectConfig {
    // Project metadata
    std::string name = "myproject";
    std::string version = "1.0.0";
    std::string description = "C++ bindings generated from Rosetta introspection";
    std::string author = "Generated";
    std::string license = "MIT";
    
    // Rosetta registration
    std::string registration_header;     // Path to the registration.h file
    std::string registration_namespace;  // Namespace containing register function
    std::string registration_function;   // Function name to call for registration
    
    // Type configuration
    std::string types_namespace;         // Namespace for bound types (e.g., "arch", "mylib")
    
    // Include configuration
    std::vector<std::string> include_dirs;   // Include directories
    std::vector<std::string> source_headers; // Headers to include in generated code
    std::vector<std::string> link_libraries; // Libraries to link against
    
    // Output configuration
    std::string output_base_dir = "./generated";
    
    // Target configurations
    TargetConfig python;
    TargetConfig wasm;
    TargetConfig javascript;
    TargetConfig rest;  // REST API server
    
    // Generator options
    bool generate_stubs = true;      // Generate .pyi for Python
    bool generate_typescript = true; // Generate .d.ts for JS/WASM
    bool generate_readme = true;     // Generate README.md for each target
    bool generate_cmake = true;      // Generate CMakeLists.txt
    
    // Advanced options
    std::vector<std::string> numpy_types;  // Types to convert to numpy arrays
    std::vector<std::string> skip_classes; // Classes to skip during generation
    std::vector<std::string> skip_methods; // Methods to skip (format: "ClassName::method")
    
    // Validation
    bool is_valid() const {
        return !name.empty() && 
               !registration_header.empty() &&
               !registration_function.empty() &&
               (python.enabled || wasm.enabled || javascript.enabled || rest.enabled);
    }
    
    std::vector<std::string> get_enabled_targets() const {
        std::vector<std::string> targets;
        if (python.enabled) targets.push_back("python");
        if (wasm.enabled) targets.push_back("wasm");
        if (javascript.enabled) targets.push_back("javascript");
        if (rest.enabled) targets.push_back("rest");
        return targets;
    }
};
