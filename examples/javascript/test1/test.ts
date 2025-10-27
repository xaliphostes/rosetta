import * as geometry from './build/Release/geometry';

// Full type safety!
const vec: geometry.Vector3D = new geometry.Vector3D();
vec.x = 3.0;

const length: number = vec.length();
const str: string = vec.to_string();

// Callbacks with types
const doubled: number[] = geometry.transformValues([1, 2, 3], (x: number): number => {
    return x * 2;
});

// Enums
const red: number = geometry.Color.Red;