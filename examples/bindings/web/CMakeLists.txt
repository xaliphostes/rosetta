# Standalone build of the auto-embind example.
#
# Prereqs:
#   1. emsdk activated (`emcmake`, `emcc` on PATH)
#   2. The emsdk's bundled clang must support C++26 reflection. The stock
#      emsdk clang does NOT — you'll need to integrate the clang-p2996
#      fork into your emsdk (replace upstream/bin/clang++ or symlink it).
#      Until that's done, configure will succeed but compilation will
#      fail with "unknown argument: -freflection".
#
# From this directory:
#   emcmake cmake -G Ninja -B build
#   cmake --build build
#   node test_reflected_person.js
# or open index.html in a browser served over http (file:// won't load the .wasm).

cmake_minimum_required(VERSION 3.28)

project(auto_emscripten_example CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR
        "This must be configured via `emcmake cmake ...`. "
        "Make sure emsdk is activated (`source <emsdk>/emsdk_env.sh`).")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(reflected_person auto_emscripten.cpp)

target_include_directories(reflected_person PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_options(reflected_person PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_options(reflected_person PRIVATE
    --bind
    -sMODULARIZE=1
    -sEXPORT_NAME=createModule
    -sENVIRONMENT=node,web
    -sALLOW_MEMORY_GROWTH=1
    -sDISABLE_EXCEPTION_CATCHING=0)

set_target_properties(reflected_person PROPERTIES
    SUFFIX ".js")

# Copy artifacts next to the JS test / HTML harness so paths "just work".
add_custom_command(TARGET reflected_person POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:reflected_person>
        ${CMAKE_CURRENT_SOURCE_DIR}/reflected_person.js
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE_DIR:reflected_person>/reflected_person.wasm
        ${CMAKE_CURRENT_SOURCE_DIR}/reflected_person.wasm)
