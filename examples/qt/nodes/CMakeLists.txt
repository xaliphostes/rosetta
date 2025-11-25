cmake_minimum_required(VERSION 3.16)
project(nodes_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Fetch nodeeditor library
include(FetchContent)
FetchContent_Declare(
    QtNodes
    GIT_REPOSITORY https://github.com/paceholder/nodeeditor.git
    GIT_TAG master
)

# Configure nodeeditor to build as static library
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)

FetchContent_MakeAvailable(QtNodes)

# Set path to Rosetta headers - adjust as needed
set(ROSETTA_INCLUDE_DIR "$ENV{HOME}/devs/c++/rosetta/include" 
    CACHE PATH "Path to Rosetta include directory")

if(NOT EXISTS "${ROSETTA_INCLUDE_DIR}/rosetta/rosetta.h")
    message(WARNING "Rosetta headers not found at ${ROSETTA_INCLUDE_DIR}")
    message(WARNING "Set ROSETTA_INCLUDE_DIR to your Rosetta include path")
    message(WARNING "You can override with: cmake -DROSETTA_INCLUDE_DIR=/path/to/rosetta/include")
else()
    message(STATUS "Found Rosetta headers at ${ROSETTA_INCLUDE_DIR}")
endif()

# Example Application
add_executable(nodes_example_demo
    main.cxx
)

target_include_directories(nodes_example_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROSETTA_INCLUDE_DIR}
)

target_link_libraries(nodes_example_demo PRIVATE
    Qt6::Core
    Qt6::Widgets
    QtNodes
)

# Set output directory
set_target_properties(nodes_example_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS nodes_example_demo
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt6 Version: ${Qt6_VERSION}")
message(STATUS "  Rosetta Include: ${ROSETTA_INCLUDE_DIR}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Build with:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "  ./bin/nodes_example_demo")
message(STATUS "")