cmake_minimum_required(VERSION 3.15)
project(rosetta_python_bindings)

set(MODULE_NAME "pyflower")

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(WIN32)
    set(Python_EXECUTABLE "C:/Python313/")
endif()

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)

# =============================================================================
# Find the dynamic flower library
# =============================================================================
# The library is built in ../lib by the flower CMakeLists.txt
set(FLOWER_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(FLOWER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../flower/include")

# Find the shared library
find_library(FLOWER_LIBRARY
    NAMES flower
    PATHS ${FLOWER_LIB_DIR}
    NO_DEFAULT_PATH
)

if(NOT FLOWER_LIBRARY)
    message(WARNING "Flower library not found in ${FLOWER_LIB_DIR}. Make sure to build the flower library first.")
    # Create an imported target anyway for the build to proceed
    add_library(flower SHARED IMPORTED)
    if(WIN32)
        set_target_properties(flower PROPERTIES
            IMPORTED_LOCATION "${FLOWER_LIB_DIR}/flower.dll"
            IMPORTED_IMPLIB "${FLOWER_LIB_DIR}/flower.lib"
        )
    elseif(APPLE)
        set_target_properties(flower PROPERTIES
            IMPORTED_LOCATION "${FLOWER_LIB_DIR}/libflower.dylib"
        )
    else()
        set_target_properties(flower PROPERTIES
            IMPORTED_LOCATION "${FLOWER_LIB_DIR}/libflower.so"
        )
    endif()
else()
    message(STATUS "Found Flower library: ${FLOWER_LIBRARY}")
    add_library(flower SHARED IMPORTED)
    set_target_properties(flower PROPERTIES
        IMPORTED_LOCATION "${FLOWER_LIBRARY}"
    )
    if(WIN32)
        # On Windows, also need the import library
        set_target_properties(flower PROPERTIES
            IMPORTED_IMPLIB "${FLOWER_LIB_DIR}/flower.lib"
        )
    endif()
endif()

set_target_properties(flower PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FLOWER_INCLUDE_DIR}"
)

# =============================================================================
# Include directories for Rosetta headers
# =============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Python_INCLUDE_DIRS}
    ../../../include
    ../../../../include
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
    ${FLOWER_INCLUDE_DIR}
)

# =============================================================================
# Link with the dynamic flower library
# =============================================================================
target_link_libraries(${MODULE_NAME} PRIVATE flower)

# Link with Python libraries if needed (Windows)
if(WIN32 AND Python_LIBRARIES)
    target_link_libraries(${MODULE_NAME} PRIVATE ${Python_LIBRARIES})
endif()

# Set output directory
set_target_properties(${MODULE_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# =============================================================================
# Handle runtime library path (RPATH)
# =============================================================================
if(NOT WIN32)
    # Set RPATH so the Python module can find the dynamic library at runtime
    set_target_properties(${MODULE_NAME} PROPERTIES
        # Use RPATH relative to the module location
        BUILD_RPATH "${FLOWER_LIB_DIR}"
        INSTALL_RPATH "$ORIGIN/../lib"
        # Don't strip RPATH during install
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

# On Windows, copy the DLL to the output directory
if(WIN32)
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FLOWER_LIB_DIR}/flower.dll"
            "${CMAKE_BINARY_DIR}/"
        COMMENT "Copying flower.dll to build directory"
    )
endif()

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

# Also copy the dynamic library to build directory for easier testing
if(NOT WIN32)
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FLOWER_LIB_DIR}/libflower${CMAKE_SHARED_LIBRARY_SUFFIX}"
            "${CMAKE_BINARY_DIR}/"
        COMMENT "Copying libflower to build directory for testing"
    )
endif()