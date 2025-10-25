const basic = require('./build/Release/basic');

{
    console.log("Vector3D")
    const v = new basic.Vector3D();
    v.x = 3.0;
    v.y = 4.0;
    v.z = 0.0;

    console.log(v.length());  // 5.0
    v.normalize();
    console.log(v.x, v.y, v.z);  // 0.6, 0.8, 0.0
}

{
    console.log("Point")
    const p = new basic.Point();
    p.x = 3;
    p.y = 4;
    console.log(p.distance());  // 5.0
}

{
    console.log("A")
    const a = new basic.A()
    a.positions = [[1, 2, 3], [4, 5, 6]]
    a.areas = [1, 2, 3, 4, 5]
    a.stress = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    a.stresses = [[1, 2, 3, 4, 5, 6, 7, 8, 9], [1, 2, 3, 4, 5, 6, 7, 8, 9]]
    console.log("areas", a.areas)
    console.log("positions", a.positions)
    console.log("stress", a.stress)
    console.log("stresses", a.stresses)
    console.log("map", a.map)
}