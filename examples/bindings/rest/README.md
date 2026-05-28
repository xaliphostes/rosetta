# REST binding

## Build

```bash
cmake -G Ninja -B build && cmake --build build 
```

## Run the server

```bash
./build/auto_rest # listening on http://127.0.0.1:8080
```

### Test the CLI
```bash
bash test_rest.sh
```

### Test in a web page

Open http://127.0.0.1:8080/ in a browser

