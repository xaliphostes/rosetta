#pragma once
#include "GeneratorConfig.h"
#include "JsBindingGypGenerator.h"
#include "JsExampleGenerator.h"
#include "JsIndexGenerator.h"
#include "JsPackageJsonGenerator.h"
#include "JsReadmeGenerator.h"
#include "JsGenerator.h"
#include "ProjectConfig.h"
#include "Pybind11Generator.h"
#include "PythonCMakeGenerator.h"
#include "PythonExampleGenerator.h"
#include "PythonPyprojectGenerator.h"
#include "PythonReadmeGenerator.h"
#include "PythonSetupPyGenerator.h"
#include "PythonStubGenerator.h"
#include "RestApiCMakeGenerator.h"
#include "RestApiExampleGenerator.h"
#include "RestApiGenerator.h"
#include "RestApiReadmeGenerator.h"
#include "TypeScriptGenerator.h"
#include "WasmCMakeGenerator.h"
#include "WasmExampleGenerator.h"
#include "WasmGenerator.h"
#include "WasmReadmeGenerator.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

// ============================================================================
// Multi-Target Binding Generator
// Generates Python, WASM, and JavaScript bindings from Rosetta metadata
// ============================================================================

class MultiTargetGenerator {
public:
    MultiTargetGenerator(const ProjectConfig &project)
        : project_(project), config_(GeneratorConfig::from_project(project)) {

        // Ensure output directory exists
        fs::create_directories(project_.output_base_dir);
    }

    void generate_all() {
        std::cout << "Generating bindings for: " << project_.name << " v" << project_.version
                  << "\n";
        std::cout << "Output directory: " << project_.output_base_dir << "\n\n";

        if (project_.python.enabled) {
            generate_python();
        }
        if (project_.wasm.enabled) {
            generate_wasm();
        }
        if (project_.javascript.enabled) {
            generate_javascript();
        }
        if (project_.rest.enabled) {
            generate_rest();
        }

        std::cout << "\n✅ All bindings generated successfully!\n";
        print_summary();
    }

    void generate_python() {
        std::string dir = get_target_dir("python", project_.python);
        fs::create_directories(dir);

        // Copy registration header to output
        copy_registration_header(dir);

        // Generate pybind11 bindings
        write_file(dir + "/generated_pybind11.cxx", [this](std::ostream &out) {
            Pybind11Generator gen(out, config_);
            gen.generate();
        });

        // Generate build files
        if (config_.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [this](std::ostream &out) {
                PythonCMakeGenerator gen(out, config_);
                gen.generate();
            });
        }

        write_file(dir + "/setup.py", [this](std::ostream &out) {
            PythonSetupPyGenerator gen(out, config_);
            gen.generate();
        });

        write_file(dir + "/pyproject.toml", [this](std::ostream &out) {
            PythonPyprojectGenerator gen(out, config_);
            gen.generate();
        });

        // Generate type stubs
        if (config_.generate_stubs) {
            write_file(dir + "/" + config_.module_name + ".pyi", [this](std::ostream &out) {
                PythonStubGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate README
        if (config_.generate_readme) {
            write_file(dir + "/README.md", [this](std::ostream &out) {
                PythonReadmeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate example
        if (config_.generate_example) {
            write_file(dir + "/example.py", [this](std::ostream &out) {
                PythonExampleGenerator gen(out, config_);
                gen.generate();
            });
        }

        std::cout << "✅ Python bindings: " << dir << "\n";
    }

    void generate_wasm() {
        std::string dir = get_target_dir("wasm", project_.wasm);
        fs::create_directories(dir);

        // Copy registration header to output
        copy_registration_header(dir);

        // Generate embind bindings
        write_file(dir + "/generated_embind.cxx", [this](std::ostream &out) {
            WasmGenerator gen(out, config_);
            gen.generate();
        });

        // Generate CMakeLists.txt
        if (config_.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [this](std::ostream &out) {
                WasmCMakeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate TypeScript declarations
        if (config_.generate_typescript) {
            write_file(dir + "/" + config_.module_name + ".d.ts", [this](std::ostream &out) {
                TypeScriptGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate README
        if (config_.generate_readme) {
            write_file(dir + "/README.md", [this](std::ostream &out) {
                WasmReadmeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate example
        if (config_.generate_example) {
            write_file(dir + "/example.js", [this](std::ostream &out) {
                WasmExampleGenerator gen(out, config_);
                gen.generate();
            });
        }

        std::cout << "✅ WASM bindings: " << dir << "\n";
    }

    void generate_javascript() {
        std::string dir = get_target_dir("javascript", project_.javascript);
        fs::create_directories(dir);

        // Copy registration header to output
        copy_registration_header(dir);

        // Generate N-API bindings
        write_file(dir + "/generated_napi.cxx", [this](std::ostream &out) {
            JsGenerator gen(out, config_);
            gen.generate();
        });

        write_file(dir + "/package.json", [this](std::ostream &out) {
            JsPackageJsonGenerator gen(out, config_);
            gen.generate();
        });

        write_file(dir + "/binding.gyp", [this](std::ostream &out) {
            JsBindingGypGenerator gen(out, config_);
            gen.generate();
        });

        write_file(dir + "/index.js", [this](std::ostream &out) {
            JsIndexGenerator gen(out, config_);
            gen.generate();
        });

        // Generate TypeScript declarations
        if (config_.generate_typescript) {
            write_file(dir + "/" + config_.module_name + ".d.ts", [this](std::ostream &out) {
                TypeScriptGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate README
        if (config_.generate_readme) {
            write_file(dir + "/README.md", [this](std::ostream &out) {
                JsReadmeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate example
        if (config_.generate_example) {
            write_file(dir + "/example.js", [this](std::ostream &out) {
                JsExampleGenerator gen(out, config_);
                gen.generate();
            });
        }

        std::cout << "✅ JavaScript bindings: " << dir << "\n";
    }

    void generate_rest() {
        std::string dir = get_target_dir("rest", project_.rest);
        fs::create_directories(dir);

        // Copy registration header to output
        copy_registration_header(dir);

        // Generate REST API server
        write_file(dir + "/generated_rest_api.cxx", [this](std::ostream &out) {
            RestApiGenerator gen(out, config_);
            gen.generate();
        });

        // Generate CMakeLists.txt
        if (config_.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [this](std::ostream &out) {
                RestApiCMakeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate README
        if (config_.generate_readme) {
            write_file(dir + "/README.md", [this](std::ostream &out) {
                RestApiReadmeGenerator gen(out, config_);
                gen.generate();
            });
        }

        // Generate example
        if (config_.generate_example) {
            write_file(dir + "/index.html", [this](std::ostream &out) {
                RestApiExampleGenerator gen(out, config_);
                gen.generate();
            });
        }

        std::cout << "✅ REST API server: " << dir << "\n";
    }

private:
    ProjectConfig   project_;
    GeneratorConfig config_;

    std::string get_target_dir(const std::string &target, const TargetConfig &tc) const {
        if (!tc.output_dir.empty()) {
            return tc.output_dir;
        }
        return project_.output_base_dir + "/" + target;
    }

    void copy_registration_header(const std::string &target_dir) {
        if (project_.registration_header.empty()) {
            return;
        }

        fs::path src(project_.registration_header);
        if (!fs::exists(src)) {
            std::cerr << "Warning: Registration header not found: " << project_.registration_header
                      << "\n";
            return;
        }

        fs::path dst = fs::path(target_dir) / src.filename();
        try {
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        } catch (const fs::filesystem_error &e) {
            std::cerr << "Warning: Could not copy registration header: " << e.what() << "\n";
        }
    }

    void write_file(const std::string &path, std::function<void(std::ostream &)> writer) {
        std::ofstream out(path);
        if (!out) {
            throw std::runtime_error("Failed to create file: " + path);
        }
        writer(out);
    }

    void print_summary() const {
        std::cout << "\nGenerated targets:\n";

        if (project_.python.enabled) {
            std::cout << "  Python:\n";
            std::cout << "    cd " << get_target_dir("python", project_.python) << "\n";
            std::cout << "    pip install .\n";
        }

        if (project_.wasm.enabled) {
            std::cout << "  WASM:\n";
            std::cout << "    cd " << get_target_dir("wasm", project_.wasm) << "\n";
            std::cout << "    emcmake cmake -B build && cmake --build build\n";
        }

        if (project_.javascript.enabled) {
            std::cout << "  JavaScript:\n";
            std::cout << "    cd " << get_target_dir("javascript", project_.javascript) << "\n";
            std::cout << "    npm install && npm run build\n";
        }

        if (project_.rest.enabled) {
            std::cout << "  REST API:\n";
            std::cout << "    cd " << get_target_dir("rest", project_.rest) << "\n";
            std::cout << "    mkdir build && cd build && cmake .. && make\n";
            std::cout << "    ./" << config_.module_name << "_server --port 8080\n";
        }
    }
};
