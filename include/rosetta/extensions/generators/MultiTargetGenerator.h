#pragma once
#include "GeneratorConfig.h"
#include "ProjectConfig.h"
#include "js/JsBindingGypGenerator.h"
#include "js/JsExampleGenerator.h"
#include "js/JsGenerator.h"
#include "js/JsIndexGenerator.h"
#include "js/JsPackageJsonGenerator.h"
#include "js/JsReadmeGenerator.h"
#include "py/PythonCMakeGenerator.h"
#include "py/PythonExampleGenerator.h"
#include "py/PythonGenerator.h"
#include "py/PythonPyprojectGenerator.h"
#include "py/PythonReadmeGenerator.h"
#include "py/PythonSetupPyGenerator.h"
#include "py/PythonStubGenerator.h"
#include "rest/RestApiCMakeGenerator.h"
#include "rest/RestApiExampleGenerator.h"
#include "rest/RestApiGenerator.h"
#include "rest/RestApiReadmeGenerator.h"
#include "ts/TypeScriptGenerator.h"
#include "wasm/WasmCMakeGenerator.h"
#include "wasm/WasmExampleGenerator.h"
#include "wasm/WasmGenerator.h"
#include "wasm/WasmReadmeGenerator.h"
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
        : project_(project), base_config_(GeneratorConfig::from_project(project)) {

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

        // Get target-specific config
        GeneratorConfig config =
            GeneratorConfig::from_project_for_target(project_, project_.python);

        copy_registration_header(dir);

        write_file(dir + "/generated_pybind11.cxx", [&config](std::ostream &out) {
            Pybind11Generator gen(out, config);
            gen.generate();
        });

        if (config.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [&config](std::ostream &out) {
                PythonCMakeGenerator gen(out, config);
                gen.generate();
            });
        }

        write_file(dir + "/setup.py", [&config](std::ostream &out) {
            PythonSetupPyGenerator gen(out, config);
            gen.generate();
        });

        write_file(dir + "/pyproject.toml", [&config](std::ostream &out) {
            PythonPyprojectGenerator gen(out, config);
            gen.generate();
        });

        if (config.generate_stubs) {
            write_file(dir + "/" + config.module_name + ".pyi", [&config](std::ostream &out) {
                PythonStubGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_readme) {
            write_file(dir + "/README.md", [&config](std::ostream &out) {
                PythonReadmeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_example) {
            write_file(dir + "/example.py", [&config](std::ostream &out) {
                PythonExampleGenerator gen(out, config);
                gen.generate();
            });
        }

        std::cout << "✔ Python bindings: " << dir << "\n";
        if (config.should_compile_sources()) {
            std::cout << "   Mode: static (compiling " << config.source_files.size()
                      << " source files)\n";
        } else {
            std::cout << "   Mode: dynamic (linking against libraries)\n";
        }
    }

    void generate_wasm() {
        std::string dir = get_target_dir("wasm", project_.wasm);
        fs::create_directories(dir);

        // WASM always needs static compilation, force it
        GeneratorConfig config = GeneratorConfig::from_project_for_target(project_, project_.wasm);

        // Override: WASM must use static compilation
        if (config.source_files.empty() && project_.sources.has_sources()) {
            std::cerr << "Warning: WASM target requires source files for static compilation.\n";
            std::cerr << "         Using global source configuration.\n";
            config.source_files = GeneratorConfig::resolve_source_files(project_.sources);
        }

        copy_registration_header(dir);

        write_file(dir + "/generated_embind.cxx", [&config](std::ostream &out) {
            WasmGenerator gen(out, config);
            gen.generate();
        });

        if (config.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [&config](std::ostream &out) {
                WasmCMakeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_typescript) {
            write_file(dir + "/" + config.module_name + ".d.ts", [&config](std::ostream &out) {
                TypeScriptGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_readme) {
            write_file(dir + "/README.md", [&config](std::ostream &out) {
                WasmReadmeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_example) {
            write_file(dir + "/example.js", [&config](std::ostream &out) {
                WasmExampleGenerator gen(out, config);
                gen.generate();
            });
        }

        std::cout << "✔ WASM bindings: " << dir << "\n";
        std::cout << "   Mode: static (compiling " << config.source_files.size()
                  << " source files)\n";
    }

    void generate_javascript() {
        std::string dir = get_target_dir("javascript", project_.javascript);
        fs::create_directories(dir);

        GeneratorConfig config =
            GeneratorConfig::from_project_for_target(project_, project_.javascript);

        copy_registration_header(dir);

        write_file(dir + "/generated_napi.cxx", [&config](std::ostream &out) {
            JsGenerator gen(out, config);
            gen.generate();
        });

        write_file(dir + "/package.json", [&config](std::ostream &out) {
            JsPackageJsonGenerator gen(out, config);
            gen.generate();
        });

        write_file(dir + "/binding.gyp", [&config](std::ostream &out) {
            JsBindingGypGenerator gen(out, config);
            gen.generate();
        });

        write_file(dir + "/index.js", [&config](std::ostream &out) {
            JsIndexGenerator gen(out, config);
            gen.generate();
        });

        if (config.generate_typescript) {
            write_file(dir + "/" + config.module_name + ".d.ts", [&config](std::ostream &out) {
                TypeScriptGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_readme) {
            write_file(dir + "/README.md", [&config](std::ostream &out) {
                JsReadmeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_example) {
            write_file(dir + "/example.js", [&config](std::ostream &out) {
                JsExampleGenerator gen(out, config);
                gen.generate();
            });
        }

        std::cout << "✔ JavaScript bindings: " << dir << "\n";
        if (config.should_compile_sources()) {
            std::cout << "   Mode: static (compiling " << config.source_files.size()
                      << " source files)\n";
        } else {
            std::cout << "   Mode: dynamic (linking against libraries)\n";
        }
    }

    void generate_rest() {
        std::string dir = get_target_dir("rest", project_.rest);
        fs::create_directories(dir);

        GeneratorConfig config = GeneratorConfig::from_project_for_target(project_, project_.rest);

        copy_registration_header(dir);

        write_file(dir + "/generated_rest_api.cxx", [&config](std::ostream &out) {
            RestApiGenerator gen(out, config);
            gen.generate();
        });

        if (config.generate_cmake) {
            write_file(dir + "/CMakeLists.txt", [&config](std::ostream &out) {
                RestApiCMakeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_readme) {
            write_file(dir + "/README.md", [&config](std::ostream &out) {
                RestApiReadmeGenerator gen(out, config);
                gen.generate();
            });
        }

        if (config.generate_example) {
            write_file(dir + "/index.html", [&config](std::ostream &out) {
                RestApiExampleGenerator gen(out, config);
                gen.generate();
            });
        }

        std::cout << "✔ REST API server: " << dir << "\n";
    }

private:
    ProjectConfig   project_;
    GeneratorConfig base_config_; // Base config (not target-specific)

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
            std::cout << "    ./" << base_config_.module_name << "_server --port 8080\n";
        }
    }
};
