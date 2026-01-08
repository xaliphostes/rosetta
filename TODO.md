# TODO list

## Rosetta
1. Deal with **enums**
2. Seems that we need at least one default ctor. If not, the call to 
    ```cpp
    auto any = class_meta.constructors()[0]({});
    ```
    will failed since no ctor are registered!

## Python generator

## JavaScript generator
todo

## Wasm generator
todo

## REST API generator
todo
