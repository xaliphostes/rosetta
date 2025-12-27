// ============================================================================
// Multi-Target Binding Generator
// Generates: pybind11, embind (WASM), N-API (Node.js), and TypeScript declarations
// Configuration-driven - reads project settings from JSON/YAML file
// ============================================================================
//
// IMPORTANT: For full binding generation, create a custom main() that calls
// your Rosetta registration function before running the generator.
// See main_example.cxx and BindingGeneratorLib.h for the recommended approach.
//
// Usage:
//   ./binding_generator <config_file>       Generate bindings from config
//   ./binding_generator --init [path]       Create sample config file
//   ./binding_generator --help              Show help
//
// Config file format (JSON):
// {
//   "project": { "name": "mylib", "version": "1.0.0", ... },
//   "rosetta": { 
//     "registration_header": "...", 
//     "registration_function": "...",
//     "types_namespace": "..."
//   },
//   "includes": { "directories": [...], "headers": [...] },
//   "targets": { "python": { "enabled": true }, ... }
// }
// ============================================================================

#include <iostream>
#include <string>
#include <cstring>

#include "ConfigParser.h"
#include "ProjectConfig.h"
#include "MultiTargetGenerator.h"

void print_usage(const char* program_name) {
    std::cout << "Multi-Target Binding Generator\n";
    std::cout << "Generates language bindings from Rosetta C++ introspection metadata\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program_name << " <config.json>        Generate bindings from config file\n";
    std::cout << "  " << program_name << " --init [path]        Create sample config file\n";
    std::cout << "  " << program_name << " --targets            Show detailed target information\n";
    std::cout << "  " << program_name << " --help               Show this help message\n";
    std::cout << "\n";
    std::cout << "Available binding targets:\n";
    std::cout << "  python      - pybind11 bindings with NumPy support\n";
    std::cout << "  wasm        - Emscripten/embind for WebAssembly\n";
    std::cout << "  javascript  - Node.js N-API native addon\n";
    std::cout << "  rest        - REST API server with JSON endpoints\n";
    std::cout << "\n";
    std::cout << "Config file format:\n";
    std::cout << "  JSON file specifying project metadata, Rosetta registration,\n";
    std::cout << "  include paths, and which targets to generate.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " project.json\n";
    std::cout << "  " << program_name << " --init myproject.json\n";
    std::cout << "\n";
    std::cout << "For more information, see README.md and USAGE.md\n";
}

void print_targets() {
    std::cout << "Available Binding Targets\n";
    std::cout << "=========================\n\n";
    
    std::cout << "PYTHON (pybind11)\n";
    std::cout << "-----------------\n";
    std::cout << "  Output files:\n";
    std::cout << "    - generated_pybind11.cxx   C++ binding code\n";
    std::cout << "    - CMakeLists.txt           CMake build configuration\n";
    std::cout << "    - setup.py                 Python setuptools config\n";
    std::cout << "    - pyproject.toml           PEP 517/518 config\n";
    std::cout << "    - <module>.pyi             Type stubs for IDE support\n";
    std::cout << "    - README.md                Usage documentation\n";
    std::cout << "  Features:\n";
    std::cout << "    - NumPy array conversion for Vector3, Matrix33, std::vector\n";
    std::cout << "    - Automatic memory management with shared_ptr\n";
    std::cout << "    - Inheritance support with proper Python MRO\n";
    std::cout << "  Build: pip install . OR cmake -B build && cmake --build build\n\n";
    
    std::cout << "WASM (Emscripten/embind)\n";
    std::cout << "------------------------\n";
    std::cout << "  Output files:\n";
    std::cout << "    - generated_embind.cxx     C++ binding code\n";
    std::cout << "    - CMakeLists.txt           CMake build configuration\n";
    std::cout << "    - <module>.d.ts            TypeScript declarations\n";
    std::cout << "    - README.md                Usage documentation\n";
    std::cout << "  Features:\n";
    std::cout << "    - TypedArray conversion (Float64Array, Int32Array)\n";
    std::cout << "    - Browser and Node.js compatible\n";
    std::cout << "    - Async module loading\n";
    std::cout << "  Build: emcmake cmake -B build && cmake --build build\n\n";
    
    std::cout << "JAVASCRIPT (Node.js N-API)\n";
    std::cout << "--------------------------\n";
    std::cout << "  Output files:\n";
    std::cout << "    - generated_napi.cxx       C++ binding code\n";
    std::cout << "    - package.json             npm package config\n";
    std::cout << "    - binding.gyp              node-gyp build config\n";
    std::cout << "    - index.js                 ES module entry point\n";
    std::cout << "    - <module>.d.ts            TypeScript declarations\n";
    std::cout << "    - README.md                Usage documentation\n";
    std::cout << "  Features:\n";
    std::cout << "    - Native addon (no WebAssembly overhead)\n";
    std::cout << "    - TypedArray conversion\n";
    std::cout << "    - ABI stable across Node.js versions\n";
    std::cout << "  Build: npm install && npm run build\n\n";
    
    std::cout << "REST API (cpp-httplib)\n";
    std::cout << "----------------------\n";
    std::cout << "  Output files:\n";
    std::cout << "    - generated_rest_api.cxx   Complete REST server\n";
    std::cout << "    - CMakeLists.txt           CMake build configuration\n";
    std::cout << "    - README.md                API documentation\n";
    std::cout << "  Endpoints:\n";
    std::cout << "    - GET  /api/classes             List registered classes\n";
    std::cout << "    - GET  /api/classes/:name       Get class info\n";
    std::cout << "    - POST /api/objects/:class      Create object\n";
    std::cout << "    - POST /api/objects/:id/:method Call method\n";
    std::cout << "    - DELETE /api/objects/:id       Delete object\n";
    std::cout << "  Features:\n";
    std::cout << "    - JSON request/response\n";
    std::cout << "    - Object lifecycle management\n";
    std::cout << "    - CORS enabled for browser clients\n";
    std::cout << "  Build: cmake -B build && cmake --build build\n";
    std::cout << "  Run:   ./<module>_server --port 8080\n";
}

void print_config_info(const ProjectConfig& config) {
    std::cout << "Configuration loaded:\n";
    std::cout << "  Project: " << config.name << " v" << config.version << "\n";
    std::cout << "  Output:  " << config.output_base_dir << "\n";
    std::cout << "  Targets: ";
    auto targets = config.get_enabled_targets();
    for (size_t i = 0; i < targets.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << targets[i];
    }
    std::cout << "\n\n";
}

int main(int argc, char* argv[]) {
    // Handle no arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string arg1 = argv[1];

    // Handle --help
    if (arg1 == "--help" || arg1 == "-h") {
        print_usage(argv[0]);
        return 0;
    }

    // Handle --targets
    if (arg1 == "--targets" || arg1 == "-t") {
        print_targets();
        return 0;
    }

    // Handle --init
    if (arg1 == "--init") {
        std::string config_path = (argc > 2) ? argv[2] : "project.json";
        try {
            ConfigParser::generate_sample_config(config_path);
            std::cout << "\nEdit this file to configure your project, then run:\n";
            std::cout << "  " << argv[0] << " " << config_path << "\n";
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    // Otherwise, treat as config file path
    std::string config_path = arg1;

    try {
        // Load configuration
        std::cout << "Loading configuration: " << config_path << "\n";
        ProjectConfig config = ConfigParser::load(config_path);

        // Validate configuration
        if (!config.is_valid()) {
            std::cerr << "Error: Invalid configuration\n";
            if (config.name.empty()) {
                std::cerr << "  - Missing project name\n";
            }
            if (config.registration_header.empty()) {
                std::cerr << "  - Missing rosetta.registration_header\n";
            }
            if (config.registration_function.empty()) {
                std::cerr << "  - Missing rosetta.registration_function\n";
            }
            if (!config.python.enabled && !config.wasm.enabled && 
                !config.javascript.enabled && !config.rest.enabled) {
                std::cerr << "  - No targets enabled (python/wasm/javascript/rest)\n";
            }
            return 1;
        }

        print_config_info(config);

        // Generate bindings
        MultiTargetGenerator generator(config);
        generator.generate_all();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
