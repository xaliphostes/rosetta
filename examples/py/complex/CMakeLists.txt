cmake_minimum_required(VERSION 3.15)
project(rosetta_python_bindings)

set(MODULE_NAME "complex")

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
pybind11_add_module(${MODULE_NAME}
    binding.cxx
)

# Set include directories for the target
target_include_directories(${MODULE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link with Python libraries if needed (Windows)
if(WIN32 AND Python_LIBRARIES)
    target_link_libraries(${MODULE_NAME} PRIVATE ${Python_LIBRARIES})
endif()

# Set output directory
set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# On Windows, ensure .pyd extension
if(WIN32)
    set_target_properties(${MODULE_NAME} PROPERTIES SUFFIX ".pyd")
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)

# =============================================================================
# Copy test script to build directory
# =============================================================================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.py)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test.py
        ${CMAKE_BINARY_DIR}/test_${MODULE_NAME}.py
        COPYONLY
    )
    message(STATUS "Copied test.py to test_${MODULE_NAME}.py in build directory")
endif()