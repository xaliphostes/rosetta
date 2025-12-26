cmake_minimum_required(VERSION 3.16)
project(binding_generator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json 3.9 QUIET)

# If nlohmann_json not found, fetch it
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Rosetta library (assumes it's in parent directory or specified via ROSETTA_DIR)
if(NOT DEFINED ROSETTA_INCLUDE_DIR)
    set(ROSETTA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
endif()

# Generator executable
add_executable(binding_generator
    main.cxx
)

target_include_directories(binding_generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROSETTA_INCLUDE_DIR}
)

target_link_libraries(binding_generator PRIVATE
    nlohmann_json::nlohmann_json
)

# Optional: YAML support
find_package(yaml-cpp QUIET)
if(yaml-cpp_FOUND)
    target_compile_definitions(binding_generator PRIVATE HAVE_YAML_CPP)
    target_link_libraries(binding_generator PRIVATE yaml-cpp)
    message(STATUS "YAML support enabled")
else()
    message(STATUS "YAML support disabled (yaml-cpp not found)")
endif()

# Installation
install(TARGETS binding_generator
    RUNTIME DESTINATION bin
)

# Install sample config
install(FILES project.json
    DESTINATION share/binding_generator
    RENAME sample_project.json
)

message(STATUS "Binding Generator configured")
message(STATUS "  Rosetta include: ${ROSETTA_INCLUDE_DIR}")