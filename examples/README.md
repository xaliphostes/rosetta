# Compiling and testing the examples

## 1. For the `all` example

This example shows, from a third party library, how to generate one Rosetta introspection.
This introspection is then use in Python, Emscripten, JavaScript... to generate the corresponding binding.
All the bindings shared the same API defined with Rosetta.

Go inside each folder and follow the README.md

## 2. For Python
Go to the `py` folder (not the sub folders) and type:

```sh
mkdir build 
cd build
cmake .. && make
make run # will run all subfolders
```

**NOTE**: The CMakeFiles.txt is configured for Python 3.14 (toward the end). Make your change...

## 3. For JavaScript (node.js)

Go inside each leaf folder, and then type:

```sh
npm i
node test.cjs
```

## 4. For Emscripten

Go to the `em/XXX` folder and type

```sh
mkdir build
cd build
emcmake cmake ..
emmake make
node test.mjs
```