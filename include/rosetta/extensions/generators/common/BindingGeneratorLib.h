#pragma once
#include "ConfigParser.h"
#include "MultiTargetGenerator.h"
#include <functional>
#include <iostream>

// ============================================================================
// Binding Generator Library
//
// This provides a library interface for the binding generator. Users should
// create their own main() that:
//   1. Calls their Rosetta registration function
//   2. Calls BindingGeneratorLib::generate()
//
// Example usage:
//
//   #include "BindingGeneratorLib.h"
//   #include "my_registration.h"  // User's registration header
//
//   int main(int argc, char* argv[]) {
//       // Initialize Rosetta with user's classes
//       my_namespace::register_my_classes();
//
//       // Run the generator
//       return BindingGeneratorLib::run(argc, argv);
//   }
//
// ============================================================================

class BindingGeneratorLib {
public:
    // Run the generator with command-line arguments
    // Call your Rosetta registration function BEFORE calling this
    static int run(int argc, char *argv[]) {
        if (argc < 2) {
            print_usage(argv[0]);
            return 1;
        }

        std::string arg1 = argv[1];

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
            return init_config(config_path, argv[0]);
        }

        return generate_from_config(arg1);
    }

    // Generate bindings from a config file
    // Call your Rosetta registration function BEFORE calling this
    static int generate_from_config(const std::string &config_path) {
        try {
            std::cout << "Loading configuration: " << config_path << "\n";
            ProjectConfig config = ConfigParser::load(config_path);

            if (!config.is_valid()) {
                print_validation_errors(config);
                return 1;
            }

            print_config_info(config);

            MultiTargetGenerator generator(config);
            generator.generate_all();

            return 0;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    // Generate bindings programmatically
    // Call your Rosetta registration function BEFORE calling this
    static int generate(const ProjectConfig &config) {
        try {
            if (!config.is_valid()) {
                print_validation_errors(config);
                return 1;
            }

            print_config_info(config);

            MultiTargetGenerator generator(config);
            generator.generate_all();

            return 0;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

private:
    static void print_usage(const char *program_name) {
        std::cout << "Multi-Target Binding Generator\n";
        std::cout << "Generates language bindings from Rosetta C++ introspection metadata\n\n";
        std::cout << "Usage:\n";
        std::cout << "  " << program_name
                  << " <config.json>        Generate bindings from config file\n";
        std::cout << "  " << program_name << " --init [path]        Create sample config file\n";
        std::cout << "  " << program_name
                  << " --targets            Show detailed target information\n";
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

    static void print_targets() {
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

    static int init_config(const std::string &config_path, const char *program_name) {
        try {
            ConfigParser::generate_sample_config(config_path);
            std::cout << "\nEdit this file to configure your project, then run:\n";
            std::cout << "  " << program_name << " " << config_path << "\n";
            return 0;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    static void print_config_info(const ProjectConfig &config) {
        std::cout << "Configuration loaded:\n";
        std::cout << "  Project: " << config.name << " v" << config.version << "\n";
        std::cout << "  Output:  " << config.output_base_dir << "\n";
        std::cout << "  Targets: ";
        auto targets = config.get_enabled_targets();
        for (size_t i = 0; i < targets.size(); ++i) {
            if (i > 0)
                std::cout << ", ";
            std::cout << targets[i];
        }
        std::cout << "\n\n";
    }

    static void print_validation_errors(const ProjectConfig &config) {
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
        if (!config.python.enabled && !config.wasm.enabled && !config.javascript.enabled) {
            std::cerr << "  - No targets enabled\n";
        }
    }
};
