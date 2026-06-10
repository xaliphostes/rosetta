# Standalone build of the auto-CxxWrap (jlcxx) example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   julia test_reflected_person.jl
#
# Requires the clang-p2996 fork at $HOME/devs/c++/clang-p2996/build and
# CxxWrap.jl / libcxxwrap-julia (the C++ side ships the JlCxx CMake
# package). Install on the Julia side with: `using Pkg; Pkg.add("CxxWrap")`.

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(auto_jlcxx_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Locate JlCxx (from libcxxwrap-julia). If it isn't hinted via JlCxx_DIR or
# CMAKE_PREFIX_PATH, ask Julia's CxxWrap for the prefix that ships the
# JlCxx CMake package.
if(NOT JlCxx_DIR AND NOT DEFINED CMAKE_PREFIX_PATH)
    execute_process(
        COMMAND julia -e "using CxxWrap; print(CxxWrap.prefix_path())"
        OUTPUT_VARIABLE CXXWRAP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_PREFIX_PATH "${CXXWRAP_PREFIX}")
endif()
find_package(JlCxx REQUIRED)

add_library(reflected_person SHARED auto_jlcxx.cpp)

target_include_directories(reflected_person PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

target_compile_options(reflected_person PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_libraries(reflected_person PRIVATE
    JlCxx::cxxwrap_julia
    JlCxx::cxxwrap_julia_stl)

# Link the fork's libc++ explicitly, mirroring the other backends. Note:
# the prebuilt libcxxwrap-julia is compiled against its own C++ runtime, so
# mixing it with the clang-p2996 fork's libc++ can surface ABI mismatches
# (similar in spirit to the pybind11 NO_EXTRAS/LTO caveat). Rebuild
# libcxxwrap-julia with the fork if you hit one.
target_link_options(reflected_person PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)
