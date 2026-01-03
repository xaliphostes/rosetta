#pragma once
#include "../common/CodeWriter.h"

// ============================================================================
// JavaScript binding.gyp Generator
// ============================================================================
class JsBindingGypGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("{");
        indent();
        line("\"targets\": [");
        indent();
        line("{");
        indent();
        
        write_target_name();
        write_sources();
        write_include_dirs();
        write_dependencies();
        write_compiler_flags();
        write_defines();
        write_library_config();
        write_platform_settings();
        write_conditions();
        
        dedent();
        line("}");
        dedent();
        line("]");
        dedent();
        line("}");
    }

private:
    void write_target_name() {
        line("\"target_name\": \"" + config_.module_name + "\",");
        line();
    }
    
    void write_sources() {
        line("\"sources\": [");
        indent();
        line("\"generated_napi.cxx\"");
        
        // Add library source files for static compilation
        if (config_.should_compile_sources() && !config_.source_files.empty()) {
            for (const auto& src : config_.source_files) {
                line(", \"" + escape_path(src) + "\"");
            }
        }
        dedent();
        line("],");
        line();
    }
    
    void write_include_dirs() {
        line("\"include_dirs\": [");
        indent();
        line("\"<!@(node -p \\\"require('node-addon-api').include\\\")\"");
        for (const auto &inc : config_.include_dirs) {
            std::string cleaned = inc;
            if (cleaned.find("${") != std::string::npos)
                continue;
            line(", \"" + escape_path(cleaned) + "\"");
        }
        dedent();
        line("],");
        line();
    }
    
    void write_dependencies() {
        line("\"dependencies\": [\"<!(node -p \\\"require('node-addon-api').gyp\\\")\"],");
        line();
    }
    
    void write_compiler_flags() {
        line("\"cflags!\": [\"-fno-exceptions\", \"-fno-rtti\"],");
        line("\"cflags_cc!\": [\"-fno-exceptions\", \"-fno-rtti\"],");
        line("\"cflags_cc\": [\"-std=c++20\", \"-fexceptions\"],");
        line();
    }
    
    void write_defines() {
        line("\"defines\": [\"NAPI_CPP_EXCEPTIONS\"],");
        line();
    }
    
    void write_library_config() {
        // Only include library config for dynamic linking mode
        if (!config_.should_link_library()) {
            return;
        }
        
        // Library directories
        if (!config_.library_dirs.empty()) {
            line("\"library_dirs\": [");
            indent();
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                std::string comma = (i < config_.library_dirs.size() - 1) ? "," : "";
                line("\"" + escape_path(config_.library_dirs[i]) + "\"" + comma);
            }
            dedent();
            line("],");
            line();
        }
        
        // Libraries to link
        if (!config_.link_libraries.empty()) {
            line("\"libraries\": [");
            indent();
            for (size_t i = 0; i < config_.link_libraries.size(); ++i) {
                std::string lib = config_.link_libraries[i];
                std::string comma = (i < config_.link_libraries.size() - 1) ? "," : "";
                // Add -l prefix if not already present and not a full path
                if (lib.find('/') == std::string::npos && 
                    lib.find('\\') == std::string::npos &&
                    lib.substr(0, 2) != "-l") {
                    lib = "-l" + lib;
                }
                line("\"" + lib + "\"" + comma);
            }
            dedent();
            line("],");
            line();
        }
    }
    
    void write_platform_settings() {
        line("\"xcode_settings\": {");
        indent();
        line("\"GCC_ENABLE_CPP_EXCEPTIONS\": \"YES\",");
        line("\"CLANG_CXX_LANGUAGE_STANDARD\": \"c++20\",");
        line("\"CLANG_CXX_LIBRARY\": \"libc++\",");
        line("\"MACOSX_DEPLOYMENT_TARGET\": \"10.15\",");
        if (config_.should_link_library() && !config_.library_dirs.empty()) {
            std::string lib_paths;
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                if (i > 0) lib_paths += " ";
                lib_paths += escape_path(config_.library_dirs[i]);
            }
            line("\"LIBRARY_SEARCH_PATHS\": [\"" + lib_paths + "\"],");
        }
        line("\"GCC_ENABLE_CPP_RTTI\": \"YES\"");
        dedent();
        line("},");
        line();
        
        line("\"msvs_settings\": {");
        indent();
        line("\"VCCLCompilerTool\": {");
        indent();
        line("\"ExceptionHandling\": 1,");
        line("\"AdditionalOptions\": [\"/std:c++20\"]");
        dedent();
        if (config_.should_link_library() && !config_.library_dirs.empty()) {
            line("},");
            line("\"VCLinkerTool\": {");
            indent();
            std::string lib_dirs;
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                if (i > 0) lib_dirs += ";";
                lib_dirs += escape_path(config_.library_dirs[i]);
            }
            line("\"AdditionalLibraryDirectories\": [\"" + lib_dirs + "\"]");
            dedent();
        }
        line("}");
        dedent();
        line("},");
        line();
    }
    
    void write_conditions() {
        line("\"conditions\": [");
        indent();
        line("[\"OS=='linux'\", {\"cflags_cc\": [\"-std=c++20\", \"-fexceptions\"]}],");
        line("[\"OS=='mac'\", {\"cflags_cc\": [\"-std=c++20\", \"-fexceptions\"]}],");
        line("[\"OS=='win'\", {\"defines\": [\"WIN32\", \"_WINDOWS\"]}]");
        dedent();
        line("]");
    }
    
    std::string escape_path(const std::string& path) const {
        std::string result = path;
        // Replace backslashes with forward slashes for JSON
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
};