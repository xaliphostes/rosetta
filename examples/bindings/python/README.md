# Python binding

## Prerequisites

```bash
pip install pybind11
```

## Build

```bash
cmake -G Ninja -B build
cmake --build build
```

## Run

```bash
python test_reflected_person.py
```