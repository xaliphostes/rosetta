#pragma once
#include "../CodeWriter.h"

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
        line("\"target_name\": \"" + config_.module_name + "\",");
        line();
        line("\"sources\": [\"generated_napi.cxx\"],");
        line();
        line("\"include_dirs\": [");
        indent();
        line("\"<!@(node -p \\\"require('node-addon-api').include\\\")\",");
        for (const auto &inc : config_.include_dirs) {
            std::string cleaned = inc;
            if (cleaned.find("${") != std::string::npos)
                continue;
            line("\"" + cleaned + "\",");
        }
        dedent();
        line("],");
        line();
        line("\"dependencies\": [\"<!(node -p "
             "\\\"require('node-addon-api').gyp\\\")\"],");
        line();
        line("\"cflags!\": [\"-fno-exceptions\", \"-fno-rtti\"],");
        line("\"cflags_cc!\": [\"-fno-exceptions\", \"-fno-rtti\"],");
        line("\"cflags_cc\": [\"-std=c++20\", \"-fexceptions\"],");
        line();
        line("\"defines\": [\"NAPI_CPP_EXCEPTIONS\"],");
        line();
        
        // Library directories
        if (!config_.library_dirs.empty()) {
            line("\"library_dirs\": [");
            indent();
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                std::string comma = (i < config_.library_dirs.size() - 1) ? "," : "";
                line("\"" + config_.library_dirs[i] + "\"" + comma);
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
        
        line("\"xcode_settings\": {");
        indent();
        line("\"GCC_ENABLE_CPP_EXCEPTIONS\": \"YES\",");
        line("\"CLANG_CXX_LANGUAGE_STANDARD\": \"c++20\",");
        line("\"CLANG_CXX_LIBRARY\": \"libc++\",");
        line("\"MACOSX_DEPLOYMENT_TARGET\": \"10.15\",");
        if (!config_.library_dirs.empty()) {
            std::string lib_paths;
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                if (i > 0) lib_paths += " ";
                lib_paths += config_.library_dirs[i];
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
        if (!config_.library_dirs.empty()) {
            line("},");
            line("\"VCLinkerTool\": {");
            indent();
            std::string lib_dirs;
            for (size_t i = 0; i < config_.library_dirs.size(); ++i) {
                if (i > 0) lib_dirs += ";";
                lib_dirs += config_.library_dirs[i];
            }
            line("\"AdditionalLibraryDirectories\": [\"" + lib_dirs + "\"]");
            dedent();
        }
        line("}");
        dedent();
        line("},");
        line();
        line("\"conditions\": [");
        indent();
        line("[\"OS=='linux'\", {\"cflags_cc\": [\"-std=c++20\", "
             "\"-fexceptions\"]}],");
        line("[\"OS=='mac'\", {\"cflags_cc\": [\"-std=c++20\", "
             "\"-fexceptions\"]}],");
        line("[\"OS=='win'\", {\"defines\": [\"WIN32\", \"_WINDOWS\"]}]");
        dedent();
        line("]");
        dedent();
        line("}");
        dedent();
        line("]");
        dedent();
        line("}");
    }
};