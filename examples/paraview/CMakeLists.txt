# Standalone build of the ParaView XML example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   ./build/paraview_xml                   # print Server Manager XML to stdout
#   ./build/paraview_xml > Threshold.xml   # capture the plugin XML

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(paraview_xml_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(paraview_xml main.cpp)

target_include_directories(paraview_xml PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_options(paraview_xml PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_options(paraview_xml PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)
