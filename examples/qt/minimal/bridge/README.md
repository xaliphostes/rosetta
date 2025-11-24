## Compile
```sh
mkdir build
cd build
```

Since we need to explicitly tells where is Qt (for QtQuick3D):
```sh
cmake -DCMAKE_PREFIX_PATH=~/Qt/6.9.1/macos/ ..
make
```

## Run (on macos)
```sh
./rosetta_3d_editor.app/Contents/MacOS/rosetta_3d_editor
```