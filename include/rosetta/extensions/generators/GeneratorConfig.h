#pragma once
#include "ProjectConfig.h"
#include <set>
#include <string>
#include <vector>

// ============================================================================
// Generator Configuration - Internal configuration used by generators
// ============================================================================

struct GeneratorConfig {
    // Project metadata
    std::string module_name = "mymodule";
    std::string version = "1.0.0";
    std::string author = "Generated";
    std::string description = "C++ bindings generated from Rosetta introspection";
    std::string license = "MIT";

    // Rosetta registration
    std::string registration_header;
    std::string registration_namespace;
    std::string registration_function;

    // Type configuration
    std::string types_namespace;  // Namespace for bound types (e.g., "arch", "mylib")
    
    // Namespace handling for generated bindings
    bool strip_namespaces = true;         // Strip C++ namespaces from binding names
    std::string namespace_separator = ""; // Separator to use instead of "::" (empty = strip entirely)

    // Include paths (relative to generated folder or absolute)
    std::vector<std::string> include_dirs;

    // Library directories (-L paths)
    std::vector<std::string> library_dirs;

    // Source headers to include in generated code
    std::vector<std::string> source_headers;

    // Libraries to link
    std::vector<std::string> link_libraries;

    // Types that should be converted to numpy arrays (Python) or TypedArrays (JS)
    std::set<std::string> numpy_types = {
        "Vector3", "Matrix33", 
        "std::vector<double>", "std::vector<float>", "std::vector<int>"
    };

    // Classes/methods to skip
    std::set<std::string> skip_classes;
    std::set<std::string> skip_methods;

    // Generator options
    bool generate_stubs = true;
    bool generate_typescript = true;
    bool generate_readme = true;
    bool generate_example = true;
    bool generate_cmake = true;

    // WASM-specific options
    bool wasm_single_file = false;
    bool wasm_export_es6 = false;
    std::string wasm_environment = "";

    // Create from ProjectConfig
    static GeneratorConfig from_project(const ProjectConfig& proj) {
        GeneratorConfig config;

        config.module_name = proj.name;
        config.version = proj.version;
        config.author = proj.author;
        config.description = proj.description;
        config.license = proj.license;

        config.registration_header = proj.registration_header;
        config.registration_namespace = proj.registration_namespace;
        config.registration_function = proj.registration_function;

        config.types_namespace = proj.types_namespace;
        config.strip_namespaces = proj.strip_namespaces;
        config.namespace_separator = proj.namespace_separator;

        config.include_dirs = proj.include_dirs;
        config.library_dirs = proj.library_dirs;
        config.source_headers = proj.source_headers;
        config.link_libraries = proj.link_libraries;

        // Convert vectors to sets for O(1) lookup
        for (const auto& t : proj.numpy_types) {
            config.numpy_types.insert(t);
        }
        for (const auto& c : proj.skip_classes) {
            config.skip_classes.insert(c);
        }
        for (const auto& m : proj.skip_methods) {
            config.skip_methods.insert(m);
        }

        config.generate_stubs = proj.generate_stubs;
        config.generate_typescript = proj.generate_typescript;
        config.generate_readme = proj.generate_readme;
        config.generate_example = proj.generate_example;
        config.generate_cmake = proj.generate_cmake;

        // WASM-specific options
        config.wasm_single_file = proj.wasm.single_file;
        config.wasm_export_es6 = proj.wasm.export_es6;
        config.wasm_environment = proj.wasm.environment;

        return config;
    }

    // Check if a class should be skipped
    bool should_skip_class(const std::string& name) const {
        return skip_classes.count(name) > 0;
    }

    // Check if a method should be skipped
    bool should_skip_method(const std::string& class_name, 
                            const std::string& method_name) const {
        return skip_methods.count(class_name + "::" + method_name) > 0 ||
               skip_methods.count(method_name) > 0;
    }

    // Check if type needs numpy/TypedArray conversion
    bool needs_array_conversion(const std::string& type) const {
        for (const auto& t : numpy_types) {
            if (type.find(t) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // Get the registration include directive
    std::string get_registration_include() const {
        if (registration_header.empty()) {
            return "";
        }
        // Extract just the filename for the include
        size_t pos = registration_header.rfind('/');
        if (pos == std::string::npos) {
            pos = registration_header.rfind('\\');
        }
        std::string filename = (pos != std::string::npos) 
            ? registration_header.substr(pos + 1) 
            : registration_header;
        return "#include \"" + filename + "\"";
    }

    // Get the full registration call
    std::string get_registration_call() const {
        std::string call;
        if (!registration_namespace.empty()) {
            call = registration_namespace + "::";
        }
        call += registration_function + "()";
        return call;
    }

    // Get a fully qualified type name with the types namespace
    std::string qualified_type(const std::string& type_name) const {
        if (types_namespace.empty()) {
            return type_name;
        }
        return types_namespace + "::" + type_name;
    }

    // Transform a C++ class name to a binding name (for Python/JS/etc.)
    // Handles namespace stripping/replacement based on configuration
    std::string binding_name(const std::string& cpp_name) const {
        if (!strip_namespaces) {
            // Keep namespaces, but replace :: with separator if specified
            if (!namespace_separator.empty()) {
                std::string result = cpp_name;
                size_t pos = 0;
                while ((pos = result.find("::", pos)) != std::string::npos) {
                    result.replace(pos, 2, namespace_separator);
                    pos += namespace_separator.length();
                }
                return result;
            }
            return cpp_name;
        }
        
        // Strip namespaces: extract just the class name after the last ::
        size_t pos = cpp_name.rfind("::");
        if (pos != std::string::npos) {
            return cpp_name.substr(pos + 2);
        }
        return cpp_name;
    }
};