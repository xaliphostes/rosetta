cmake_minimum_required(VERSION 3.15)
project(rosetta_python_bindings)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
if(WIN32)
    # Option 1: Set Python path explicitly
    set(Python_EXECUTABLE "C:/Python313/")
    
    # # Option 2: Use py launcher to find Python
    # execute_process(
    #     COMMAND py -3 -c "import sys; print(sys.executable)"
    #     OUTPUT_VARIABLE PYTHON_FROM_PY
    #     #message("Detected Python executable: ${PYTHON_FROM_PY}" )
    #     OUTPUT_STRIP_TRAILING_WHITESPACE
    #     ERROR_QUIET
    # )
endif()
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)


# =============================================================================
# Include directories for Rosetta headers
# =============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Python_INCLUDE_DIRS}
    ../../../include
)

# =============================================================================
# Create the Python module
# =============================================================================
pybind11_add_module(rosetta_example
    binding.cxx
)

# Set include directories for the target
target_include_directories(rosetta_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link with Python libraries if needed (Windows)
if(WIN32 AND Python_LIBRARIES)
    target_link_libraries(rosetta_example PRIVATE ${Python_LIBRARIES})
endif()

# Set output directory
set_target_properties(rosetta_example PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# On Windows, ensure .pyd extension
if(WIN32)
    set_target_properties(rosetta_example PROPERTIES SUFFIX ".pyd")
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS rosetta_example
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)

# =============================================================================
# Copy test script to build directory
# =============================================================================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.py)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test.py
        ${CMAKE_BINARY_DIR}/test.py
        COPYONLY
    )
    message(STATUS "Copied test.py to build directory")
endif()

# =============================================================================
# Print configuration summary
# =============================================================================
message(STATUS "")
message(STATUS "=== Configuration Summary ===")
message(STATUS "Python executable: ${Python_EXECUTABLE}")
message(STATUS "Python version: ${PYTHON_VERSION_STRING}")
message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")
message(STATUS "Python libraries: ${Python_LIBRARIES}")
if(pybind11_VERSION)
    message(STATUS "pybind11 version: ${pybind11_VERSION}")
endif()
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "=============================")
message(STATUS "")