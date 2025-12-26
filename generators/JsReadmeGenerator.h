#pragma once
#include "CodeWriter.h"

// ============================================================================
// JavaScript README Generator
// ============================================================================
class JsReadmeGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("# " + config_.module_name + " - Node.js N-API Bindings");
        line();
        line("N-API bindings generated from Rosetta introspection.");
        line();
        line("## Build");
        line();
        line("```bash");
        line("npm install");
        line("npm run build");
        line("```");
        line();
        line("## Usage");
        line();
        line("```javascript");
        line("import " + config_.module_name + " from './" +
             config_.module_name + "';");
        line();
        line("console.log(" + config_.module_name + ".listClasses());");
        line("```");
    }
};
