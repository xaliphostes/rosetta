# TODO list

## Rosetta
1. Deal with **enums**
2. Seems that we need at least one default ctor. If not, the call to 
    ```cpp
    auto any = class_meta.constructors()[0]({});
    ```
    will failed since no ctor are registered!

## Python generator
1. Bug in abstract classes (see idl/example).
The bug is not in Rosetta but in the generators (for instance for python). See `unittests/abstract_test.cxx` for Rosetta.

## JavaScript generator
