cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(introspection-cpp26 CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_compile_options(
    -freflection
    -freflection-latest
    -fexperimental-library
)

add_link_options(
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi
)

add_executable(hello hello.cpp)
add_executable(reflected reflected.cpp)
add_executable(overloads overloads.cpp)
add_executable(inheritance inheritance.cpp)
add_executable(annotations annotations.cpp)
target_compile_options(annotations PRIVATE -fannotation-attributes)

add_executable(docgen docgen.cpp)
target_include_directories(docgen PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_options(docgen PRIVATE -fannotation-attributes)

add_executable(reflection serialization.cpp)

add_executable(moc moc.cpp)
target_include_directories(moc PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_options(moc PRIVATE -fannotation-attributes)
