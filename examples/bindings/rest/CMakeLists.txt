# Standalone build of the auto-REST example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   ./build/auto_rest          # listens on 127.0.0.1:8080
#   bash test_rest.sh          # exercises the endpoints
#
# Pulls cpp-httplib and nlohmann/json via FetchContent — no system deps.

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(auto_rest_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(FetchContent)
FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.1)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3)
FetchContent_MakeAvailable(httplib json)

add_executable(auto_rest auto_rest.cpp)

target_include_directories(auto_rest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_link_libraries(auto_rest PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json)

target_compile_options(auto_rest PRIVATE
    -freflection
    -freflection-latest
    -fexperimental-library
    -fannotation-attributes)

target_link_options(auto_rest PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)
