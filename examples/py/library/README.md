# General example

Shows how to bind a third party library using `Rosetta` for Python.

## Folder flower

The third party library that we cannot modify. A static library is generated. 

## Folder pyflower

The Rosetta introspection + python binding. Only the headers of the library `flower` are used

## Compilation

- Go to the **flower** folder, create a `build` directory and run `cmake .. && make`

- Go to the **pyflower** folder, create a `build` directory and run `cmake .. && make`

## Testing

In the folder **pyflower/build**, since the script `test.py` is copied, run
```sh
python test.py
```
