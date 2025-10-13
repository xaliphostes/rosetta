const geometry = require('./build/Release/basic');

const v = new geometry.Vector3D();
v.x = 3.0;
v.y = 4.0;
v.z = 0.0;

console.log(v.length());  // 5.0
v.normalize();
console.log(v.x, v.y, v.z);  // 0.6, 0.8, 0.0

const p = new geometry.Point2D();
p.x = 3;
p.y = 4;
console.log(p.distance());  // 5.0
