#pragma once
#include "CodeWriter.h"

// ============================================================================
// JavaScript package.json Generator
// ============================================================================
class JsPackageJsonGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("{");
        indent();
        line("\"name\": \"" + config_.module_name + "\",");
        line("\"version\": \"" + config_.version + "\",");
        line("\"description\": \"" + config_.description + "\",");
        line("\"main\": \"index.js\",");
        line("\"types\": \"" + config_.module_name + ".d.ts\",");
        line("\"type\": \"module\",");
        line("\"scripts\": {");
        indent();
        line("\"install\": \"node-gyp rebuild\",");
        line("\"build\": \"node-gyp configure build\",");
        line("\"clean\": \"node-gyp clean\",");
        line("\"test\": \"node test.js\"");
        dedent();
        line("},");
        line("\"keywords\": [\"rosetta\", \"introspection\", \"napi\", "
             "\"bindings\"],");
        line("\"author\": \"" + config_.author + "\",");
        line("\"license\": \"" + config_.license + "\",");
        line("\"dependencies\": {");
        indent();
        line("\"node-addon-api\": \"^7.0.0\"");
        dedent();
        line("},");
        line("\"devDependencies\": {");
        indent();
        line("\"node-gyp\": \"^10.0.0\"");
        dedent();
        line("},");
        line("\"gypfile\": true");
        dedent();
        line("}");
    }
};
