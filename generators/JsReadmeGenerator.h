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
        line("import binding from './index.js';");
        line();
        line("const model = new binding.Model();");
        line();
        line("const positions = new Float64Array([0, 0, 0, 1, 0, 0, 0, 1, 0]);");
        line("const indices = new Int32Array([0, 1, 2]);");
        line("const surface = new binding.Surface(positions, indices);");
        line("model.addSurface(surface);");
        line();
        line("surface.points.forEach((p, i) => console.log(`Point ${i}: ${p.x} ${p.y} ${p.z}`))");
        line("surface.triangles.forEach((t, i) => console.log(`Triangle ${i}: ${t.a} ${t.b} "
             "${t.c}`))");
        line();
        line("surface.transform((p) => {");
        line("    return new binding.Point(p.x * 2 + 1, p.y * 2 + 1, p.z * 2 + 1);");
        line("});");
        line("surface.points.forEach((p, i) => console.log(`Point ${i}: ${p.x} ${p.y} ${p.z}`))");
        line("```");
        line();
        line("**Expected output:**");
        line();
        line("```");
        line("Point 0: 0 0 0");
        line("Point 1: 1 0 0");
        line("Point 2: 0 1 0");
        line("Triangle 0: 0 1 2");
        line("Point 0: 1 1 1");
        line("Point 1: 3 1 1");
        line("Point 2: 1 3 1");
        line("```");
    }
};
