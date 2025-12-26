#pragma once
#include "CodeWriter.h"

// ============================================================================
// JavaScript index.js Generator
// ============================================================================
class JsIndexGenerator : public CodeWriter {
  public:
    using CodeWriter::CodeWriter;

    void generate() override {
        line("// "
             "================================================================="
             "===========");
        line("// AUTO-GENERATED INDEX FILE - DO NOT EDIT");
        line("// "
             "================================================================="
             "===========");
        line();
        line("import { createRequire } from 'module';");
        line("const require = createRequire(import.meta.url);");
        line();
        line("const binding = require('./build/Release/" + config_.module_name +
             ".node');");
        line();
        line("export default binding;");
        line("export const { listClasses, getClassMethods } = binding;");
    }
};
