#pragma once
#include "../CodeWriter.h"

// ============================================================================
// JavaScript README Generator
// ============================================================================
class JsExampleGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
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
    }
};
