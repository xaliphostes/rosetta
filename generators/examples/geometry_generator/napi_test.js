import binding from './index.js';

const model = new binding.Model();

const surface = new binding.Surface(new Float64Array([0, 0, 0, 1, 0, 0, 0, 1, 0]), new Int32Array([0, 1, 2]));
model.addSurface(surface);

surface.points.forEach((p, i) => console.log(`Point ${i}: ${p.x} ${p.y} ${p.z}`))
surface.triangles.forEach((t, i) => console.log(`Triangle ${i}: ${t.a} ${t.b} ${t.c}`))

surface.transform((p) => {
    return new binding.Point(p.x * 2 + 1, p.y * 2 + 1, p.z * 2 + 1);
});
surface.points.forEach((p, i) => console.log(`Point ${i}: ${p.x} ${p.y} ${p.z}`))