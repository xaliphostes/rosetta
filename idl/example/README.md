## Generate
```sh
python ../rosetta_idl_parser.py geometry.yaml -o output/ --python
```

## Compile
```sh
cd output
mkdir build
cmake ..
make
```

## Testing
```sh
python ../test.py
```