## Generate
From the `example` folder
```sh
python3 ../rosetta_idl_parser.py geometry.yaml -o output/ --python
```

## Compile
```sh
cd output
mkdir build && cd build
cmake ..
make
```

## Testing
Stay in the `build` folder and type (only once)
```sh
# copy the test file in the build folder
# (for simplicity)
cp ../../test.py .
```

Then
```sh
python3 test.py
```