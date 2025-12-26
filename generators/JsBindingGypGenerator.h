#pragma once
#include "CodeWriter.h"

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
        line("\"xcode_settings\": {");
        indent();
        line("\"GCC_ENABLE_CPP_EXCEPTIONS\": \"YES\",");
        line("\"CLANG_CXX_LANGUAGE_STANDARD\": \"c++20\",");
        line("\"CLANG_CXX_LIBRARY\": \"libc++\",");
        line("\"MACOSX_DEPLOYMENT_TARGET\": \"10.15\",");
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
