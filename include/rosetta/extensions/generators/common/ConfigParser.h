#pragma once
#include "ProjectConfig.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

// ============================================================================
// Configuration File Parser
// Supports JSON format (YAML support can be added with yaml-cpp)
// ============================================================================

namespace fs = std::filesystem;

class ConfigParser {
public:
    static ProjectConfig load(const std::string &filepath) {
        if (!fs::exists(filepath)) {
            throw std::runtime_error("Config file not found: " + filepath);
        }

        std::string ext = fs::path(filepath).extension().string();

        if (ext == ".json") {
            return load_json(filepath);
        } else if (ext == ".yaml" || ext == ".yml") {
            return load_yaml(filepath);
        } else {
            throw std::runtime_error("Unsupported config format: " + ext + " (use .json or .yaml)");
        }
    }

    static LinkMode parse_link_mode(const std::string &mode_str) {
        if (mode_str == "static")
            return LinkMode::Static;
        if (mode_str == "dynamic")
            return LinkMode::Dynamic;
        if (mode_str == "both")
            return LinkMode::Both;
        throw std::runtime_error("Invalid link mode: " + mode_str +
                                 " (use 'static', 'dynamic', or 'both')");
    }

    static void parse_source_config(const nlohmann::json &j, SourceConfig &src,
                                    const fs::path &config_dir,
                                    bool set_default_base_dir = true) {
        if (j.contains("mode")) {
            src.mode = parse_link_mode(j["mode"].get<std::string>());
        }

        get_if_exists(j, "base_dir", src.base_dir);
        if (src.base_dir.empty()) {
            // Only set default base_dir for main sources, not for target-specific sources
            // This allows from_project_for_target to use the global base_dir as fallback
            if (set_default_base_dir) {
                src.base_dir = config_dir.string();
            }
        } else if (!fs::path(src.base_dir).is_absolute()) {
            src.base_dir = (config_dir / src.base_dir).string();
        }

        get_array_if_exists(j, "files", src.files);
        get_array_if_exists(j, "glob_patterns", src.glob_patterns);
        get_array_if_exists(j, "exclude_patterns", src.exclude_patterns);

        // Resolve relative file paths
        for (auto &file : src.files) {
            if (!fs::path(file).is_absolute()) {
                file = (fs::path(src.base_dir) / file).string();
            }
        }
    }

private:
    // Variable storage for ${VAR} substitution
    using VariableMap = std::map<std::string, std::string>;

    // Substitute ${VAR} patterns in a string
    static std::string substitute_variables(const std::string &input, const VariableMap &vars) {
        std::string result = input;
        size_t      pos    = 0;

        while ((pos = result.find("${", pos)) != std::string::npos) {
            size_t end = result.find("}", pos);
            if (end == std::string::npos)
                break;

            std::string var_name = result.substr(pos + 2, end - pos - 2);
            auto        it       = vars.find(var_name);

            if (it != vars.end()) {
                result.replace(pos, end - pos + 1, it->second);
                // Don't advance pos - replacement might contain more variables
            } else {
                // Variable not found - leave as-is and move past it
                pos = end + 1;
            }
        }

        return result;
    }

    // Substitute variables in a JSON value (recursively)
    static void substitute_json_variables(nlohmann::json &j, const VariableMap &vars) {
        if (j.is_string()) {
            j = substitute_variables(j.get<std::string>(), vars);
        } else if (j.is_array()) {
            for (auto &item : j) {
                substitute_json_variables(item, vars);
            }
        } else if (j.is_object()) {
            for (auto &[key, value] : j.items()) {
                substitute_json_variables(value, vars);
            }
        }
    }

    // Parse variables section
    // Priority: 1) Environment variable with same name, 2) JSON value
    // This allows overriding any variable via environment without editing the JSON
    // Example: ROSETTA_ROOT=/my/local/rosetta ./pmp_generator project.json
    static VariableMap parse_variables(const nlohmann::json &j) {
        VariableMap vars;

        if (j.contains("variables") && j["variables"].is_object()) {
            for (auto &[key, value] : j["variables"].items()) {
                if (value.is_string()) {
                    // Check if environment variable with same name exists (takes priority)
                    const char *env_val = std::getenv(key.c_str());
                    if (env_val) {
                        vars[key] = env_val;
                    } else {
                        vars[key] = value.get<std::string>();
                    }
                }
            }
        }

        // Also support explicit environment variable references: ${env:VAR}
        for (auto &[key, value] : vars) {
            if (value.length() >= 6 && value.substr(0, 6) == "${env:") {
                size_t end = value.find("}");
                if (end != std::string::npos) {
                    std::string env_var = value.substr(6, end - 6);
                    const char *env_val = std::getenv(env_var.c_str());
                    if (env_val) {
                        vars[key] = env_val;
                    }
                }
            }
        }

        return vars;
    }

    static ProjectConfig load_json(const std::string &filepath) {
        std::ifstream file(filepath);
        if (!file) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }

        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error &e) {
            throw std::runtime_error("JSON parse error: " + std::string(e.what()));
        }

        // Parse and substitute variables
        VariableMap vars = parse_variables(j);
        substitute_json_variables(j, vars);

        return parse_config(j, filepath);
    }

    static ProjectConfig load_yaml(const std::string &filepath) {
        // YAML support requires yaml-cpp library
        // For now, we'll convert simple YAML to JSON-like structure
        // In production, use yaml-cpp for proper YAML parsing
        throw std::runtime_error("YAML support requires yaml-cpp library. "
                                 "Please use JSON format or install yaml-cpp and rebuild.");
    }

    static ProjectConfig parse_config(const nlohmann::json &j, const std::string &config_path) {
        ProjectConfig config;
        // Make config_path absolute first, then get parent directory
        // This handles the case where config_path is relative (e.g., "project.json")
        fs::path      abs_config_path = fs::absolute(config_path);
        fs::path      config_dir = abs_config_path.parent_path();

        // Project metadata
        if (j.contains("project")) {
            auto &proj = j["project"];
            get_if_exists(proj, "name", config.name);
            get_if_exists(proj, "version", config.version);
            get_if_exists(proj, "description", config.description);
            get_if_exists(proj, "author", config.author);
            get_if_exists(proj, "license", config.license);
        }

        // Rosetta configuration
        if (j.contains("rosetta")) {
            auto &ros = j["rosetta"];
            get_if_exists(ros, "registration_header", config.registration_header);
            get_if_exists(ros, "registration_namespace", config.registration_namespace);
            get_if_exists(ros, "registration_function", config.registration_function);
            get_if_exists(ros, "types_namespace", config.types_namespace);

            // Namespace handling options
            if (ros.contains("cpp_namespaces")) {
                auto &ns = ros["cpp_namespaces"];
                get_if_exists(ns, "strip", config.strip_namespaces);
                get_if_exists(ns, "separator", config.namespace_separator);
            }

            // Make registration_header path relative to config file
            if (!config.registration_header.empty() &&
                !fs::path(config.registration_header).is_absolute()) {
                config.registration_header = (config_dir / config.registration_header).string();
            }
        }

        // Include configuration
        if (j.contains("includes")) {
            auto &inc = j["includes"];
            get_array_if_exists(inc, "directories", config.include_dirs);
            get_array_if_exists(inc, "library_directories", config.library_dirs);
            get_array_if_exists(inc, "headers", config.source_headers);
            get_array_if_exists(inc, "libraries", config.link_libraries);

            // Parse include glob patterns
            if (inc.contains("glob_patterns") && inc["glob_patterns"].is_array()) {
                for (const auto& gp : inc["glob_patterns"]) {
                    IncludeGlobPattern pattern;
                    get_if_exists(gp, "base_dir", pattern.base_dir);
                    get_if_exists(gp, "pattern", pattern.pattern);
                    if (!pattern.base_dir.empty() && !fs::path(pattern.base_dir).is_absolute()) {
                        pattern.base_dir = (config_dir / pattern.base_dir).string();
                    }
                    config.include_globs.push_back(pattern);
                }
            }

            // Resolve relative paths
            resolve_paths(config.include_dirs, config_dir);
            resolve_paths(config.library_dirs, config_dir);
        }

        // Parse C++ preprocessor definitions
        if (j.contains("defines") && j["defines"].is_array()) {
            for (const auto &def : j["defines"]) {
                DefineConfig define_cfg;
                if (def.is_string()) {
                    // Simple flag: "DEBUG" -> #define DEBUG
                    define_cfg.name = def.get<std::string>();
                } else if (def.is_object()) {
                    // Object with name and optional value:
                    // {"name": "VERSION", "value": "\"1.0\""} -> #define VERSION "1.0"
                    get_if_exists(def, "name", define_cfg.name);
                    get_if_exists(def, "value", define_cfg.value);
                }
                if (!define_cfg.name.empty()) {
                    config.defines.push_back(define_cfg);
                }
            }
        }

        if (j.contains("sources")) {
            parse_source_config(j["sources"], config.sources, config_dir);
        }

        // Output configuration
        if (j.contains("output")) {
            auto &out = j["output"];
            get_if_exists(out, "base_dir", config.output_base_dir);

            // Resolve relative output path
            if (!fs::path(config.output_base_dir).is_absolute()) {
                config.output_base_dir = (config_dir / config.output_base_dir).string();
            }
        }

        // Target configurations
        if (j.contains("targets")) {
            auto &targets = j["targets"];

            if (targets.contains("python")) {
                parse_target_config(targets["python"], config.python, config_dir);
            }
            if (targets.contains("wasm")) {
                parse_target_config(targets["wasm"], config.wasm, config_dir);
            }
            if (targets.contains("javascript")) {
                parse_target_config(targets["javascript"], config.javascript, config_dir);
            }
            if (targets.contains("rest")) {
                parse_target_config(targets["rest"], config.rest, config_dir);
            }
        }

        // Generator options
        if (j.contains("options")) {
            auto &opts = j["options"];
            get_if_exists(opts, "generate_stubs", config.generate_stubs);
            get_if_exists(opts, "generate_typescript", config.generate_typescript);
            get_if_exists(opts, "generate_readme", config.generate_readme);
            get_if_exists(opts, "generate_cmake", config.generate_cmake);
        }

        // Advanced options
        if (j.contains("advanced")) {
            auto &adv = j["advanced"];
            get_array_if_exists(adv, "numpy_types", config.numpy_types);
            get_array_if_exists(adv, "skip_classes", config.skip_classes);
            get_array_if_exists(adv, "skip_methods", config.skip_methods);
        }

        return config;
    }

    static void parse_target_config(const nlohmann::json &j, TargetConfig &target,
                                    const fs::path &config_dir = fs::path()) {
        get_if_exists(j, "enabled", target.enabled);
        get_if_exists(j, "output_dir", target.output_dir);
        get_array_if_exists(j, "extra_sources", target.extra_sources);
        get_array_if_exists(j, "extra_libs", target.extra_libs);

        // Python-specific options
        get_if_exists(j, "python_executable", target.python_executable);

        // WASM-specific options
        get_if_exists(j, "single_file", target.single_file);
        get_if_exists(j, "export_es6", target.export_es6);
        get_if_exists(j, "environment", target.environment);

        if (j.contains("link_mode")) {
            target.link_mode_override = parse_link_mode(j["link_mode"].get<std::string>());
        }

        if (j.contains("sources")) {
            parse_source_config(j["sources"], target.target_sources, config_dir, false);
        }
    }

    template <typename T>
    static void get_if_exists(const nlohmann::json &j, const std::string &key, T &value) {
        if (j.contains(key)) {
            value = j[key].get<T>();
        }
    }

    static void get_array_if_exists(const nlohmann::json &j, const std::string &key,
                                    std::vector<std::string> &value) {
        if (j.contains(key) && j[key].is_array()) {
            value.clear();
            for (const auto &item : j[key]) {
                value.push_back(item.get<std::string>());
            }
        }
    }

    static void resolve_paths(std::vector<std::string> &paths, const fs::path &base_dir) {
        for (auto &path : paths) {
            if (!fs::path(path).is_absolute()) {
                path = (base_dir / path).string();
            }
        }
    }

public:
    // Generate a sample config file
    static void generate_sample_config(const std::string &filepath) {
        nlohmann::json j;

        // Variables section
        j["variables"] = {{"PROJECT_ROOT", "/path/to/your/project"},
                          {"ROSETTA_ROOT", "/path/to/rosetta"}};

        j["project"] = {{"name", "myproject"},
                        {"version", "1.0.0"},
                        {"description", "C++ library with auto-generated bindings"},
                        {"author", "Your Name"},
                        {"license", "MIT"}};

        j["rosetta"] = {{"registration_header", "${PROJECT_ROOT}/src/bindings/registration.h"},
                        {"registration_namespace", "myproject_rosetta"},
                        {"registration_function", "register_classes"},
                        {"types_namespace", "myproject"},
                        {"cpp_namespaces", {{"strip", true}, {"separator", ""}}}};

        // NEW: Source configuration section
        j["sources"] = {
            {"mode", "dynamic"}, // "dynamic", "static", or "both"
            {"base_dir", "${PROJECT_ROOT}"},
            {"files", nlohmann::json::array({"src/core/Model.cpp", "src/core/Solver.cpp"})},
            {"glob_patterns", nlohmann::json::array({"src/**/*.cpp", "src/**/*.cxx"})},
            {"exclude_patterns", nlohmann::json::array({"**/test/**", "**/*_test.cpp",
                                                        "**/examples/**", "**/main.cpp"})}};

        j["includes"] = {
            {"directories",
             {"${PROJECT_ROOT}/include", "${PROJECT_ROOT}/src", "${ROSETTA_ROOT}/include"}},
            {"library_directories", {"${PROJECT_ROOT}/build/lib"}},
            {"headers",
             {"myproject/core/Types.h", "myproject/core/Model.h", "myproject/core/Solver.h"}},
            {"libraries", {"myproject_core"}}};

        // C++ preprocessor definitions
        // Can be simple strings (flag macros) or objects with name/value
        j["defines"] = nlohmann::json::array({
            "DEBUG",                                               // Simple flag: #define DEBUG
            {{"name", "VERSION"}, {"value", "\"1.0.0\""}},         // With value: #define VERSION "1.0.0"
            {{"name", "MAX_BUFFER_SIZE"}, {"value", "4096"}}       // Numeric: #define MAX_BUFFER_SIZE 4096
        });

        j["output"] = {{"base_dir", "./generated"}};

        j["targets"] = {{"python",
                         {{"enabled", true},
                          {"link_mode", "dynamic"}, // Override global mode for this target
                          {"python_executable", ""}, // Optional: Python root dir or executable (e.g., "/Library/Frameworks/Python.framework/Versions/3.12")
                          {"output_dir", ""},
                          {"extra_sources", nlohmann::json::array()},
                          {"extra_libs", nlohmann::json::array()}}},
                        {"wasm",
                         {{"enabled", true},
                          {"link_mode", "static"}, // WASM typically needs static
                          {"output_dir", ""},
                          {"extra_sources", nlohmann::json::array()},
                          {"extra_libs", nlohmann::json::array()},
                          {"single_file", true},
                          {"export_es6", false},
                          {"environment", "web,node"},
                          // Per-target source override example:
                          {"sources",
                           {{"glob_patterns", nlohmann::json::array({"src/**/*.cpp"})},
                            {"exclude_patterns", nlohmann::json::array({"**/test/**"})}}}}},
                        {"javascript",
                         {{"enabled", false},
                          {"link_mode", "static"},
                          {"output_dir", ""},
                          {"extra_sources", nlohmann::json::array()},
                          {"extra_libs", nlohmann::json::array()}}},
                        {"rest",
                         {{"enabled", false},
                          {"output_dir", ""},
                          {"extra_sources", nlohmann::json::array()},
                          {"extra_libs", nlohmann::json::array()}}}};

        j["options"] = {{"generate_stubs", true},
                        {"generate_typescript", true},
                        {"generate_readme", true},
                        {"generate_example", true},
                        {"generate_cmake", true}};

        j["advanced"] = {{"numpy_types", {"Vector3", "Matrix33", "std::vector<double>"}},
                         {"skip_classes", nlohmann::json::array()},
                         {"skip_methods", nlohmann::json::array()}};

        std::ofstream file(filepath);
        if (!file) {
            throw std::runtime_error("Cannot create config file: " + filepath);
        }
        file << j.dump(4);

        std::cout << "Sample config generated: " << filepath << "\n";
        std::cout << "\nNew 'sources' section allows:\n";
        std::cout << "  - mode: 'dynamic' (link library), 'static' (compile sources), 'both'\n";
        std::cout << "  - files: explicit list of source files\n";
        std::cout << "  - glob_patterns: recursive patterns like 'src/**/*.cpp'\n";
        std::cout << "  - exclude_patterns: patterns to exclude from glob results\n";
        std::cout << "\nPer-target overrides:\n";
        std::cout << "  - link_mode: override global mode for specific target\n";
        std::cout << "  - sources: additional/override source config per target\n";
        std::cout << "\nC++ preprocessor definitions ('defines' section):\n";
        std::cout << "  - Simple string: \"DEBUG\" -> #define DEBUG\n";
        std::cout << "  - Object with value: {\"name\": \"VERSION\", \"value\": \"\\\"1.0\\\"\"} -> #define VERSION \"1.0\"\n";
        std::cout << "  - Numeric value: {\"name\": \"MAX_SIZE\", \"value\": \"100\"} -> #define MAX_SIZE 100\n";
    }
};