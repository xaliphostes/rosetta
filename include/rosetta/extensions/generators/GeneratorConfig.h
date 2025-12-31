#pragma once
#include "ProjectConfig.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ============================================================================
// Generator Configuration - Internal configuration used by generators
// ============================================================================

struct GeneratorConfig {
    // Project metadata
    std::string module_name = "mymodule";
    std::string version     = "1.0.0";
    std::string author      = "Generated";
    std::string description = "C++ bindings generated from Rosetta introspection";
    std::string license     = "MIT";

    // Rosetta registration
    std::string registration_header;
    std::string registration_namespace;
    std::string registration_function;

    // Type configuration
    std::string types_namespace;

    // Namespace handling
    bool        strip_namespaces    = true;
    std::string namespace_separator = "";

    // Include paths
    std::vector<std::string> include_dirs;
    std::vector<std::string> library_dirs;
    std::vector<std::string> source_headers;
    std::vector<std::string> link_libraries;

    // Source compilation settings
    LinkMode                 link_mode = LinkMode::Dynamic;
    std::vector<std::string> source_files; // Resolved list of all source files

    // Types for array conversion
    std::set<std::string> numpy_types = {"Vector3", "Matrix33", "std::vector<double>",
                                         "std::vector<float>", "std::vector<int>"};

    // Classes/methods to skip
    std::set<std::string> skip_classes;
    std::set<std::string> skip_methods;

    // Generator options
    bool generate_stubs      = true;
    bool generate_typescript = true;
    bool generate_readme     = true;
    bool generate_example    = true;
    bool generate_cmake      = true;

    // WASM-specific options
    bool        wasm_single_file = false;
    bool        wasm_export_es6  = false;
    std::string wasm_environment = "";

    // ========================================================================
    // Factory methods
    // ========================================================================

    // Create base config from ProjectConfig (no target-specific overrides)
    static GeneratorConfig from_project(const ProjectConfig &proj) {
        GeneratorConfig config;

        config.module_name = proj.name;
        config.version     = proj.version;
        config.author      = proj.author;
        config.description = proj.description;
        config.license     = proj.license;

        config.registration_header    = proj.registration_header;
        config.registration_namespace = proj.registration_namespace;
        config.registration_function  = proj.registration_function;

        config.types_namespace     = proj.types_namespace;
        config.strip_namespaces    = proj.strip_namespaces;
        config.namespace_separator = proj.namespace_separator;

        config.include_dirs   = proj.include_dirs;
        config.library_dirs   = proj.library_dirs;
        config.source_headers = proj.source_headers;
        config.link_libraries = proj.link_libraries;

        // Source configuration
        config.link_mode    = proj.sources.mode;
        config.source_files = resolve_source_files(proj.sources);

        // Convert vectors to sets
        for (const auto &t : proj.numpy_types) {
            config.numpy_types.insert(t);
        }
        for (const auto &c : proj.skip_classes) {
            config.skip_classes.insert(c);
        }
        for (const auto &m : proj.skip_methods) {
            config.skip_methods.insert(m);
        }

        config.generate_stubs      = proj.generate_stubs;
        config.generate_typescript = proj.generate_typescript;
        config.generate_readme     = proj.generate_readme;
        config.generate_example    = proj.generate_example;
        config.generate_cmake      = proj.generate_cmake;

        // WASM-specific (from global wasm config)
        config.wasm_single_file = proj.wasm.single_file;
        config.wasm_export_es6  = proj.wasm.export_es6;
        config.wasm_environment = proj.wasm.environment;

        return config;
    }

    // Create target-specific config with link mode and source overrides
    static GeneratorConfig from_project_for_target(const ProjectConfig &proj,
                                                   const TargetConfig  &target) {
        GeneratorConfig config = from_project(proj);

        // Apply target-specific link mode
        config.link_mode = target.get_link_mode(proj.sources.mode);

        // Resolve source files based on effective link mode
        if (config.link_mode == LinkMode::Static || config.link_mode == LinkMode::Both) {
            // Start with global sources if target doesn't have its own
            if (target.has_target_sources()) {
                // Use target-specific sources (with global base_dir as fallback)
                SourceConfig effective_sources = target.target_sources;
                if (effective_sources.base_dir.empty()) {
                    effective_sources.base_dir = proj.sources.base_dir;
                }
                config.source_files = resolve_source_files(effective_sources);
            } else {
                // Use global sources
                config.source_files = resolve_source_files(proj.sources);
            }

            // Add any extra_sources (legacy list of explicit files)
            for (const auto &src : target.extra_sources) {
                config.source_files.push_back(src);
            }

            // Remove duplicates
            std::sort(config.source_files.begin(), config.source_files.end());
            config.source_files.erase(
                std::unique(config.source_files.begin(), config.source_files.end()),
                config.source_files.end());
        } else {
            // Dynamic mode - no source files to compile
            config.source_files.clear();
        }

        // Apply target-specific WASM options
        if (&target == &proj.wasm) {
            config.wasm_single_file = target.single_file;
            config.wasm_export_es6  = target.export_es6;
            config.wasm_environment = target.environment;
        }

        return config;
    }

    // ========================================================================
    // Helper methods
    // ========================================================================

    bool should_compile_sources() const {
        return (link_mode == LinkMode::Static || link_mode == LinkMode::Both) &&
               !source_files.empty();
    }

    bool should_link_library() const {
        return link_mode == LinkMode::Dynamic || link_mode == LinkMode::Both;
    }

    bool should_skip_class(const std::string &name) const { return skip_classes.count(name) > 0; }

    bool should_skip_method(const std::string &class_name, const std::string &method_name) const {
        return skip_methods.count(class_name + "::" + method_name) > 0 ||
               skip_methods.count(method_name) > 0;
    }

    bool needs_array_conversion(const std::string &type) const {
        for (const auto &t : numpy_types) {
            if (type.find(t) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    std::string get_registration_include() const {
        if (registration_header.empty()) {
            return "";
        }
        size_t pos = registration_header.rfind('/');
        if (pos == std::string::npos) {
            pos = registration_header.rfind('\\');
        }
        std::string filename =
            (pos != std::string::npos) ? registration_header.substr(pos + 1) : registration_header;
        return "#include \"" + filename + "\"";
    }

    std::string get_registration_call() const {
        std::string call;
        if (!registration_namespace.empty()) {
            call = registration_namespace + "::";
        }
        call += registration_function + "()";
        return call;
    }

    std::string qualified_type(const std::string &type_name) const {
        if (types_namespace.empty()) {
            return type_name;
        }
        return types_namespace + "::" + type_name;
    }

    std::string binding_name(const std::string &cpp_name) const {
        if (!strip_namespaces) {
            if (!namespace_separator.empty()) {
                std::string result = cpp_name;
                size_t      pos    = 0;
                while ((pos = result.find("::", pos)) != std::string::npos) {
                    result.replace(pos, 2, namespace_separator);
                    pos += namespace_separator.length();
                }
                return result;
            }
            return cpp_name;
        }

        size_t pos = cpp_name.rfind("::");
        if (pos != std::string::npos) {
            return cpp_name.substr(pos + 2);
        }
        return cpp_name;
    }

    // ========================================================================
    // Source file resolution (public for MultiTargetGenerator)
    // ========================================================================

    static std::vector<std::string> resolve_source_files(const SourceConfig &src) {
        std::vector<std::string> result;

        if (src.base_dir.empty()) {
            std::cerr << "Warning: source base_dir is empty, skipping source resolution\n";
            return result;
        }

        fs::path base(src.base_dir);
        if (!fs::exists(base)) {
            std::cerr << "Warning: source base_dir does not exist: " << src.base_dir << "\n";
            return result;
        }

        // Add explicit files
        for (const auto &file : src.files) {
            fs::path file_path(file);
            if (!file_path.is_absolute()) {
                file_path = base / file;
            }
            if (fs::exists(file_path)) {
                result.push_back(fs::canonical(file_path).string());
            } else {
                std::cerr << "Warning: source file not found: " << file_path << "\n";
            }
        }

        // Process glob patterns
        for (const auto &pattern : src.glob_patterns) {
            auto files = glob_files(base, pattern, src.exclude_patterns);
            result.insert(result.end(), files.begin(), files.end());
        }

        // Remove duplicates
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());

        std::cout << "  Resolved " << result.size() << " source files from " << src.base_dir
                  << "\n";

        return result;
    }

private:
    // ========================================================================
    // Glob implementation
    // ========================================================================

    static std::vector<std::string> glob_files(const fs::path &base_dir, const std::string &pattern,
                                               const std::vector<std::string> &excludes) {
        std::vector<std::string> result;

        std::string regex_pattern = glob_to_regex(pattern);
        std::regex  re;
        try {
            re = std::regex(regex_pattern, std::regex::ECMAScript | std::regex::icase);
        } catch (const std::regex_error &e) {
            std::cerr << "Warning: invalid glob pattern '" << pattern << "': " << e.what() << "\n";
            return result;
        }

        std::vector<std::regex> exclude_regexes;
        for (const auto &excl : excludes) {
            try {
                exclude_regexes.emplace_back(glob_to_regex(excl),
                                             std::regex::ECMAScript | std::regex::icase);
            } catch (const std::regex_error &e) {
                std::cerr << "Warning: invalid exclude pattern '" << excl << "': " << e.what()
                          << "\n";
            }
        }

        try {
            for (const auto &entry : fs::recursive_directory_iterator(base_dir)) {
                if (!entry.is_regular_file())
                    continue;

                std::string rel_path = fs::relative(entry.path(), base_dir).string();

                // Normalize path separators for matching
                std::replace(rel_path.begin(), rel_path.end(), '\\', '/');

                // Check if matches pattern
                if (!std::regex_match(rel_path, re))
                    continue;

                // Check excludes
                bool excluded = false;
                for (const auto &excl_re : exclude_regexes) {
                    if (std::regex_match(rel_path, excl_re)) {
                        excluded = true;
                        break;
                    }
                }

                if (!excluded) {
                    result.push_back(fs::canonical(entry.path()).string());
                }
            }
        } catch (const fs::filesystem_error &e) {
            std::cerr << "Warning: filesystem error during glob: " << e.what() << "\n";
        }

        return result;
    }

    static std::string glob_to_regex(const std::string &glob) {
        std::string regex;
        regex.reserve(glob.size() * 2);

        for (size_t i = 0; i < glob.size(); ++i) {
            char c = glob[i];
            switch (c) {
            case '*':
                if (i + 1 < glob.size() && glob[i + 1] == '*') {
                    // ** matches any path including subdirectories
                    regex += ".*";
                    ++i;
                    // Skip following / if present
                    if (i + 1 < glob.size() && (glob[i + 1] == '/' || glob[i + 1] == '\\')) {
                        ++i;
                    }
                } else {
                    // * matches anything except path separator
                    regex += "[^/\\\\]*";
                }
                break;
            case '?':
                regex += "[^/\\\\]";
                break;
            case '.':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '+':
            case '|':
            case '\\':
                regex += '\\';
                regex += c;
                break;
            default:
                regex += c;
            }
        }

        return "^" + regex + "$";
    }
};