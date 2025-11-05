//const rosetta = require('./build/Debug/rosetta');
import rosetta from './build/Debug/rosetta.node'
const pause = require('./pause');

await pause();  // attend une touche

const hr = (t) => {
    console.log('\n' + '='.repeat(30) + ` ${t} ` + '='.repeat(30));
};

hr('Sanity check: exported symbols');
console.log(Object.keys(rosetta));

/* ------------------------------------------------------------------ *
 * Vector3D
 * ------------------------------------------------------------------ */
hr('Vector3D');
const v = new rosetta.Vector3D(3, 4, 12);
console.log('constructed:', { x: v.x, y: v.y, z: v.z });
console.log('length():', v.length());

v.x = 6;
v.y = 8;
console.log('after set x=6,y=8:', { x: v.x, y: v.y, z: v.z });
console.log('new length():', v.length());

v.normalize();
console.log('after normalize():', { x: v.x, y: v.y, z: v.z });
console.log('normalized length():', v.length());

const v2 = new rosetta.Vector3D(1, 2, 3);
const v3 = v.add(v2);
console.log('v.add({1,2,3}) ->', { x: v3.x, y: v3.y, z: v3.z });

try {
    v.z = 'oops';
    console.log('type check (expected error) -> z:', v.z);
} catch (e) {
    console.log('correctly rejected wrong type for z:', e.message);
}

/* ------------------------------------------------------------------ *
 * Rectangle
 * ------------------------------------------------------------------ */
hr('Rectangle');
const r = new rosetta.Rectangle(5, 7);
console.log('constructed:', { width: r.width, height: r.height });
console.log('area():', r.area());
console.log('perimeter():', r.perimeter());

r.width = 10;
r.height = 2.5;
console.log('after resize:', { width: r.width, height: r.height });
console.log('area():', r.area(), 'perimeter():', r.perimeter());

/* ------------------------------------------------------------------ *
 * Person (properties + methods)
 * ------------------------------------------------------------------ */
hr('Person');
const p = new rosetta.Person('Ada', 37);
console.log('constructed:', { name: p.name, age: p.age });
console.log('greet():', p.greet());

p.age = 38;
p.name = 'Ada Lovelace';
console.log('after edits:', { name: p.name, age: p.age });
console.log('greet():', p.greet());

/* ------------------------------------------------------------------ *
 * Summary
 * ------------------------------------------------------------------ */
hr('Summary');
console.log('All checks finished. If no errors above, the binding works.');
