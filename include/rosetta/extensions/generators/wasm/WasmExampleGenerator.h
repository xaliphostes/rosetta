#pragma once
#include "../CodeWriter.h"

// ============================================================================
// WASM README Generator
// ============================================================================
class WasmExampleGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("import create" + config_.module_name + "Module from './" + config_.module_name +
             ".js';");
        line();
        line("const module = await create" + config_.module_name + "Module();");
        line("console.log(module.listClasses());");
        line();
        line("/* Usage (Browser)");
        line();
        line("<script src=\"" + config_.module_name + "js.js\"></script>");
        line("<script>");
        line("  " + config_.module_name + "Module().then(m => console.log(m.listClasses()));");
        line("</script>");
        line("*/");
    }
};
