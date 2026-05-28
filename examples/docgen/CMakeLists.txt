# Standalone build of the auto-docgen example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   ./build/auto_docgen              # print Markdown to stdout
#   ./build/auto_docgen > Person.md  # capture to a file

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(auto_docgen_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(auto_docgen main.cpp)

target_include_directories(auto_docgen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_options(auto_docgen PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_options(auto_docgen PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)
