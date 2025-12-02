cmake_minimum_required(VERSION 3.15)
project(dynamic_lib_example)

# Build the flower dynamic library first
add_subdirectory(flower)

# Then build the Python bindings
add_subdirectory(pyflower)

# Ensure flower is built before pyflower
add_dependencies(pyflower flower)