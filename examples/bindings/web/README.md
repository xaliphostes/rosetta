# Emscripten binding

The emsdk's bundled clang does not yet ship with the P2996 reflection patches. Configure will succeed, but compilation will  fail on -freflection until you swap emsdk's upstream/bin/clang++ with the p2996 fork. This is the same kind of toolchain hack the other examples needed once — just at a different layer.

Once Emscripten ships with reflection-capable clang (post C++26 ratification), the example will build out of the box without any swap.

## Build

```bash
cd examples/web
emcmake cmake -G Ninja -B build
cmake --build build
```

## Run locally

```bash
node test_reflected_person.js
```

# Serve and open index.html:

```bash
python3 -m http.server   # http://localhost:8000/index.html
```
