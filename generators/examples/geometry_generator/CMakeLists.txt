# ============================================================================
# Alternative: Simple CMakeLists.txt to place in your project root
# 
# Place this file as "CMakeLists_generator.txt" in your project root,
# alongside project.json, and create my_generator.cxx there as well.
# ============================================================================

cmake_minimum_required(VERSION 3.16)
project(geometry_binding_generator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Paths - adjust these to your structure
# ============================================================================
message("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
set(BINDING_GENERATOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/binding_generator")
set(ROSETTA_INCLUDE_DIR "../../../include")

# ============================================================================
# Dependencies
# ============================================================================

find_package(nlohmann_json 3.9 QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# ============================================================================
# Generator executable
# ============================================================================

add_executable(my_binding_generator
    geometry_generator.cxx
)

target_include_directories(my_binding_generator PRIVATE
    ${BINDING_GENERATOR_DIR}
    ${ROSETTA_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # For third/ and bindings/
)

target_link_libraries(my_binding_generator PRIVATE
    nlohmann_json::nlohmann_json
)

# ============================================================================
# Usage:
#   mkdir build_generator && cd build_generator
#   cmake .. -f ../CMakeLists_generator.txt
#   make
#   ./my_binding_generator ../project.json
# ============================================================================
