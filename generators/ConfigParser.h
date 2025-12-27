#pragma once
#include "ProjectConfig.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <filesystem>

// ============================================================================
// Configuration File Parser
// Supports JSON format (YAML support can be added with yaml-cpp)
// ============================================================================

namespace fs = std::filesystem;

class ConfigParser {
public:
    static ProjectConfig load(const std::string& filepath) {
        if (!fs::exists(filepath)) {
            throw std::runtime_error("Config file not found: " + filepath);
        }
        
        std::string ext = fs::path(filepath).extension().string();
        
        if (ext == ".json") {
            return load_json(filepath);
        } else if (ext == ".yaml" || ext == ".yml") {
            return load_yaml(filepath);
        } else {
            throw std::runtime_error("Unsupported config format: " + ext + 
                                     " (use .json or .yaml)");
        }
    }

private:
    static ProjectConfig load_json(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file) {
            throw std::runtime_error("Cannot open config file: " + filepath);
        }
        
        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("JSON parse error: " + std::string(e.what()));
        }
        
        return parse_config(j, filepath);
    }
    
    static ProjectConfig load_yaml(const std::string& filepath) {
        // YAML support requires yaml-cpp library
        // For now, we'll convert simple YAML to JSON-like structure
        // In production, use yaml-cpp for proper YAML parsing
        throw std::runtime_error(
            "YAML support requires yaml-cpp library. "
            "Please use JSON format or install yaml-cpp and rebuild."
        );
    }
    
    static ProjectConfig parse_config(const nlohmann::json& j, 
                                       const std::string& config_path) {
        ProjectConfig config;
        fs::path config_dir = fs::path(config_path).parent_path();
        
        // Project metadata
        if (j.contains("project")) {
            auto& proj = j["project"];
            get_if_exists(proj, "name", config.name);
            get_if_exists(proj, "version", config.version);
            get_if_exists(proj, "description", config.description);
            get_if_exists(proj, "author", config.author);
            get_if_exists(proj, "license", config.license);
        }
        
        // Rosetta configuration
        if (j.contains("rosetta")) {
            auto& ros = j["rosetta"];
            get_if_exists(ros, "registration_header", config.registration_header);
            get_if_exists(ros, "registration_namespace", config.registration_namespace);
            get_if_exists(ros, "registration_function", config.registration_function);
            get_if_exists(ros, "types_namespace", config.types_namespace);
            
            // Make registration_header path relative to config file
            if (!config.registration_header.empty() && 
                !fs::path(config.registration_header).is_absolute()) {
                config.registration_header = 
                    (config_dir / config.registration_header).string();
            }
        }
        
        // Include configuration
        if (j.contains("includes")) {
            auto& inc = j["includes"];
            get_array_if_exists(inc, "directories", config.include_dirs);
            get_array_if_exists(inc, "headers", config.source_headers);
            get_array_if_exists(inc, "libraries", config.link_libraries);
            
            // Resolve relative paths
            resolve_paths(config.include_dirs, config_dir);
        }
        
        // Output configuration
        if (j.contains("output")) {
            auto& out = j["output"];
            get_if_exists(out, "base_dir", config.output_base_dir);
            
            // Resolve relative output path
            if (!fs::path(config.output_base_dir).is_absolute()) {
                config.output_base_dir = 
                    (config_dir / config.output_base_dir).string();
            }
        }
        
        // Target configurations
        if (j.contains("targets")) {
            auto& targets = j["targets"];
            
            if (targets.contains("python")) {
                parse_target_config(targets["python"], config.python);
            }
            if (targets.contains("wasm")) {
                parse_target_config(targets["wasm"], config.wasm);
            }
            if (targets.contains("javascript")) {
                parse_target_config(targets["javascript"], config.javascript);
            }
            if (targets.contains("rest")) {
                parse_target_config(targets["rest"], config.rest);
            }
        }
        
        // Generator options
        if (j.contains("options")) {
            auto& opts = j["options"];
            get_if_exists(opts, "generate_stubs", config.generate_stubs);
            get_if_exists(opts, "generate_typescript", config.generate_typescript);
            get_if_exists(opts, "generate_readme", config.generate_readme);
            get_if_exists(opts, "generate_cmake", config.generate_cmake);
        }
        
        // Advanced options
        if (j.contains("advanced")) {
            auto& adv = j["advanced"];
            get_array_if_exists(adv, "numpy_types", config.numpy_types);
            get_array_if_exists(adv, "skip_classes", config.skip_classes);
            get_array_if_exists(adv, "skip_methods", config.skip_methods);
        }
        
        return config;
    }
    
    static void parse_target_config(const nlohmann::json& j, TargetConfig& target) {
        get_if_exists(j, "enabled", target.enabled);
        get_if_exists(j, "output_dir", target.output_dir);
        get_array_if_exists(j, "extra_sources", target.extra_sources);
        get_array_if_exists(j, "extra_libs", target.extra_libs);
    }
    
    template<typename T>
    static void get_if_exists(const nlohmann::json& j, const std::string& key, T& value) {
        if (j.contains(key)) {
            value = j[key].get<T>();
        }
    }
    
    static void get_array_if_exists(const nlohmann::json& j, const std::string& key,
                                    std::vector<std::string>& value) {
        if (j.contains(key) && j[key].is_array()) {
            value.clear();
            for (const auto& item : j[key]) {
                value.push_back(item.get<std::string>());
            }
        }
    }
    
    static void resolve_paths(std::vector<std::string>& paths, const fs::path& base_dir) {
        for (auto& path : paths) {
            if (!fs::path(path).is_absolute()) {
                path = (base_dir / path).string();
            }
        }
    }

public:
    // Generate a sample config file
    static void generate_sample_config(const std::string& filepath) {
        nlohmann::json j;
        
        j["project"] = {
            {"name", "myproject"},
            {"version", "1.0.0"},
            {"description", "C++ library with auto-generated bindings"},
            {"author", "Your Name"},
            {"license", "MIT"}
        };
        
        j["rosetta"] = {
            {"registration_header", "src/bindings/registration.h"},
            {"registration_namespace", "myproject_rosetta"},
            {"registration_function", "register_classes"},
            {"types_namespace", "myproject"}
        };
        
        j["includes"] = {
            {"directories", {"include", "src", "../external"}},
            {"headers", {
                "myproject/core/Types.h",
                "myproject/core/Model.h",
                "myproject/core/Solver.h"
            }},
            {"libraries", {"myproject_core"}}
        };
        
        j["output"] = {
            {"base_dir", "./generated"}
        };
        
        j["targets"] = {
            {"python", {
                {"enabled", true},
                {"output_dir", ""},
                {"extra_sources", nlohmann::json::array()},
                {"extra_libs", nlohmann::json::array()}
            }},
            {"wasm", {
                {"enabled", true},
                {"output_dir", ""},
                {"extra_sources", nlohmann::json::array()},
                {"extra_libs", nlohmann::json::array()}
            }},
            {"javascript", {
                {"enabled", false},
                {"output_dir", ""},
                {"extra_sources", nlohmann::json::array()},
                {"extra_libs", nlohmann::json::array()}
            }},
            {"rest", {
                {"enabled", false},
                {"output_dir", ""},
                {"extra_sources", nlohmann::json::array()},
                {"extra_libs", nlohmann::json::array()}
            }}
        };
        
        j["options"] = {
            {"generate_stubs", true},
            {"generate_typescript", true},
            {"generate_readme", true},
            {"generate_cmake", true}
        };
        
        j["advanced"] = {
            {"numpy_types", {"Vector3", "Matrix33", "std::vector<double>"}},
            {"skip_classes", nlohmann::json::array()},
            {"skip_methods", nlohmann::json::array()}
        };
        
        std::ofstream file(filepath);
        if (!file) {
            throw std::runtime_error("Cannot create config file: " + filepath);
        }
        file << j.dump(2);
        
        std::cout << "Sample config generated: " << filepath << "\n";
    }
};
