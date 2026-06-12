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

# GoogleTest is built from source (FetchContent) AFTER the options above so it
# shares this toolchain's libc++ / -fexperimental-library ABI. A prebuilt gtest
# (e.g. from Homebrew) is compiled against a different libc++ and would not link.
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(docgen docgen.cpp)
target_include_directories(docgen PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_options(docgen PRIVATE -fannotation-attributes)

add_executable(moc moc.cpp)
target_include_directories(moc PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_options(moc PRIVATE -fannotation-attributes)
target_link_libraries(moc PRIVATE GTest::gtest_main)
gtest_discover_tests(moc)
