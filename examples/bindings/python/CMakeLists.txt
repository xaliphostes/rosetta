# Standalone build of the auto-pybind11 example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   python3 test_reflected_person.py
#
# Requires the clang-p2996 fork at $HOME/devs/c++/clang-p2996/build
# and pybind11 installed (e.g. `pip install pybind11`).

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(auto_pybind_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Locate pybind11. If it's installed via pip, hint cmake at python3.
if(NOT pybind11_DIR)
    execute_process(
        COMMAND python3 -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
find_package(pybind11 REQUIRED CONFIG)

# NO_EXTRAS disables LTO, strip, visibility=hidden, etc. Required because
# pybind11_add_module force-adds `-flto`, and Apple's system linker
# (libLTO 17) can't parse bitcode produced by the clang-p2996 fork (LLVM 21+).
pybind11_add_module(reflected_person NO_EXTRAS auto_pybind.cpp)

target_include_directories(reflected_person PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_options(reflected_person PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_options(reflected_person PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)

# Place the .so right next to the Python script so it can be imported
# without setting PYTHONPATH. Copying after build is more robust than
# LIBRARY_OUTPUT_DIRECTORY, which doesn't always cover BUNDLE outputs on macOS.
add_custom_command(TARGET reflected_person POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:reflected_person>
        ${CMAKE_CURRENT_SOURCE_DIR}/$<TARGET_FILE_NAME:reflected_person>)
