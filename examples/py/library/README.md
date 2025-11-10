# General example

Shows how to bind a third party library using Rosetta for Python.

## Folder flower

The third party library that we cannot modify.

## Folder pyflower

The Rosetta introspection + python binding.

## Compilation

- Go to the **flower** folder, create a `build` directory and run `cmake .. && make`

- Go to the **pyflower** folder, create a `build` directory and run `cmake .. && make`

## Testing

In the folder **pyflowre/build**, run
```sh
python test.py
```