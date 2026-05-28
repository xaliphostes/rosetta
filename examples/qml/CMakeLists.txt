# Standalone build of the auto-QML example.
#
# From this directory:
#   cmake -G Ninja -B build && cmake --build build
#   ./build/auto_qml
#
# Requires:
#   - The clang-p2996 fork at $HOME/devs/c++/clang-p2996/build
#   - Qt 6 (defaults to $HOME/Qt/6.5.3/macos; override with -DQT_DIR=...)

cmake_minimum_required(VERSION 3.28)

set(CLANG_P2996_ROOT "$ENV{HOME}/devs/c++/clang-p2996/build"
    CACHE PATH "Bloomberg clang-p2996 fork build directory")

set(QT_DIR "$ENV{HOME}/Qt/6.5.3/macos"
    CACHE PATH "Qt 6 install prefix")

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER   "${CLANG_P2996_ROOT}/bin/clang")
    set(CMAKE_CXX_COMPILER "${CLANG_P2996_ROOT}/bin/clang++")
endif()

project(auto_qml_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# moc / rcc need to run on the wrapper QObject header.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(PREPEND CMAKE_PREFIX_PATH "${QT_DIR}")
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(auto_qml
    main.cpp
    reflected_object.h
)

qt_add_qml_module(auto_qml
    URI ReflectedQml
    VERSION 1.0
    QML_FILES Main.qml
)

target_include_directories(auto_qml PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_link_libraries(auto_qml PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2)

# Reflection flags are only needed on TUs that include <rosetta/walk.h>.
# Scoping them to the source file keeps moc-generated code away from
# experimental compiler features.
set_source_files_properties(main.cpp PROPERTIES
    COMPILE_OPTIONS "-freflection;-freflection-latest;-fexperimental-library;-fannotation-attributes;-Wno-deprecated-declarations")

target_link_options(auto_qml PRIVATE
    -nostdlib++
    -L${CLANG_P2996_ROOT}/lib
    -Wl,-rpath,${CLANG_P2996_ROOT}/lib
    -lc++
    -lc++abi)
