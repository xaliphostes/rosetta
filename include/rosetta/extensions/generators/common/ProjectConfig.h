#pragma once
#include <string>
#include <vector>
#include <set>

// ============================================================================
// Project Configuration - Loaded from JSON/YAML config file
// ============================================================================

// Source compilation mode
enum class LinkMode {
    Dynamic,    // Link against pre-built library (current behavior)
    Static,     // Compile source files directly into binding
    Both        // Support both modes (generate both configurations)
};

struct SourceConfig {
    LinkMode mode = LinkMode::Dynamic;
    
    // Explicit list of source files (relative to config file or absolute)
    std::vector<std::string> files;
    
    // Glob patterns for recursive file discovery
    std::vector<std::string> glob_patterns;
    
    // Patterns to exclude from glob results
    std::vector<std::string> exclude_patterns;
    
    // Base directory for relative paths and glob patterns
    std::string base_dir;
    
    // Helper to check if we need to compile sources
    bool has_sources() const {
        return !files.empty() || !glob_patterns.empty();
    }
    
    // Helper to check if static compilation is enabled
    bool is_static() const {
        return mode == LinkMode::Static || mode == LinkMode::Both;
    }
    
    // Helper to check if dynamic linking is enabled
    bool is_dynamic() const {
        return mode == LinkMode::Dynamic || mode == LinkMode::Both;
    }
};

// Include directory glob pattern for discovering modular include directories
struct IncludeGlobPattern {
    std::string base_dir;    // Base directory to search from
    std::string pattern;     // Glob pattern (e.g., "*/include")
};

struct TargetConfig {
    bool enabled = false;
    std::string output_dir;  // Override default output directory
    std::vector<std::string> extra_sources;  // Additional source files
    std::vector<std::string> extra_libs;     // Additional libraries to link

    // Python-specific: path to Python root directory or executable
    // Examples: "/Library/Frameworks/Python.framework/Versions/3.12" (macOS)
    //           "/usr/bin/python3.11" (Linux executable)
    // If set, CMake will use this to find the specific Python version
    std::string python_executable;

    // Per-target link mode override (optional)
    // If not set, uses global sources.mode
    std::optional<LinkMode> link_mode_override;
    
    // Per-target additional source files (merged with global)
    SourceConfig target_sources;
    
    // WASM-specific options
    bool single_file = false;        // Embed WASM binary in JS file
    bool export_es6 = false;         // Generate ES6 module
    std::string environment = "";    // Target environment: "web", "node", "web,node"
    
    // Get effective link mode (target override or global)
    LinkMode get_link_mode(LinkMode global_mode) const {
        return link_mode_override.value_or(global_mode);
    }

    // Check if this target has its own source configuration
    bool has_target_sources() const {
        return target_sources.has_sources();
    }
};

// struct TargetConfig {
//     bool enabled = false;
//     std::string output_dir;  // Override default output directory
//     std::vector<std::string> extra_sources;  // Additional source files
//     std::vector<std::string> extra_libs;     // Additional libraries to link
    
//     // WASM-specific options
//     bool single_file = false;        // Embed WASM binary in JS file
//     bool export_es6 = false;         // Generate ES6 module
//     std::string environment = "";    // Target environment: "web", "node", "web,node"
// };

struct ProjectConfig {
    // Project metadata
    std::string name = "myproject";
    std::string version = "1.0.0";
    std::string description = "C++ bindings generated from Rosetta introspection";
    std::string author = "Generated";
    std::string license = "MIT";

    // Sources config
    SourceConfig sources;
    
    // Rosetta registration
    std::string registration_header;     // Path to the registration.h file
    std::string registration_namespace;  // Namespace containing register function
    std::string registration_function;   // Function name to call for registration
    
    // Type configuration
    std::string types_namespace;         // Namespace for bound types (e.g., "arch", "mylib")
    
    // Namespace handling for generated bindings
    bool strip_namespaces = true;        // Strip C++ namespaces from binding names (default: true)
    std::string namespace_separator = ""; // Separator to use instead of "::" (empty = strip entirely)
    
    // Include configuration
    std::vector<std::string> include_dirs;            // Explicit include directories
    std::vector<IncludeGlobPattern> include_globs;    // Glob patterns for include dirs
    std::vector<std::string> library_dirs;            // Library directories (-L paths)
    std::vector<std::string> source_headers;          // Headers to include in generated code
    std::vector<std::string> link_libraries;          // Libraries to link against
    
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
    bool generate_example = true;     // Generate an example for each target
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