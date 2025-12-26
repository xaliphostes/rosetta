#pragma once
#include "CodeWriter.h"

// ============================================================================
// WASM README Generator
// ============================================================================
class WasmReadmeGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("# " + config_.module_name + " - WebAssembly Bindings");
        line();
        line("Emscripten/WASM bindings generated from Rosetta introspection.");
        line();
        line("## Build");
        line();
        line("```bash");
        line("mkdir build && cd build");
        line("emcmake cmake ..");
        line("emmake make");
        line("```");
        line();
        line("## Usage (Node.js)");
        line();
        line("```javascript");
        line("import create" + config_.module_name + "Module from './" +
             config_.module_name + "js.js';");
        line();
        line("const module = await create" + config_.module_name + "Module();");
        line("console.log(module.listClasses());");
        line("```");
        line();
        line("## Usage (Browser)");
        line();
        line("```html");
        line("<script src=\"" + config_.module_name + "js.js\"></script>");
        line("<script>");
        line("  " + config_.module_name +
             "Module().then(m => console.log(m.listClasses()));");
        line("</script>");
        line("```");
    }
};
