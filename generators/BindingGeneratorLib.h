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
    static int run(int argc, char* argv[]) {
        if (argc < 2) {
            print_usage(argv[0]);
            return 1;
        }

        std::string arg1 = argv[1];

        if (arg1 == "--help" || arg1 == "-h") {
            print_usage(argv[0]);
            return 0;
        }

        if (arg1 == "--init") {
            std::string config_path = (argc > 2) ? argv[2] : "project.json";
            return init_config(config_path, argv[0]);
        }

        return generate_from_config(arg1);
    }

    // Generate bindings from a config file
    // Call your Rosetta registration function BEFORE calling this
    static int generate_from_config(const std::string& config_path) {
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
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    // Generate bindings programmatically
    // Call your Rosetta registration function BEFORE calling this
    static int generate(const ProjectConfig& config) {
        try {
            if (!config.is_valid()) {
                print_validation_errors(config);
                return 1;
            }

            print_config_info(config);

            MultiTargetGenerator generator(config);
            generator.generate_all();

            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

private:
    static void print_usage(const char* program_name) {
        std::cout << "Multi-Target Binding Generator\n";
        std::cout << "Generates language bindings from Rosetta C++ introspection metadata\n\n";
        std::cout << "Usage:\n";
        std::cout << "  " << program_name << " <config.json>        Generate bindings from config file\n";
        std::cout << "  " << program_name << " --init [path]        Create sample config file\n";
        std::cout << "  " << program_name << " --help               Show this help message\n";
        std::cout << "\n";
        std::cout << "IMPORTANT: Your Rosetta registration function must be called before\n";
        std::cout << "           running this generator. See BindingGeneratorLib.h for details.\n";
    }

    static int init_config(const std::string& config_path, const char* program_name) {
        try {
            ConfigParser::generate_sample_config(config_path);
            std::cout << "\nEdit this file to configure your project, then run:\n";
            std::cout << "  " << program_name << " " << config_path << "\n";
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    static void print_config_info(const ProjectConfig& config) {
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

    static void print_validation_errors(const ProjectConfig& config) {
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
