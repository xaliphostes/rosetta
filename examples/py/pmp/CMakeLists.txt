cmake_minimum_required(VERSION 3.15)
project(pmppy VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# # Find Python and pybind11
# find_package(Python COMPONENTS Interpreter Development REQUIRED)
# find_package(pybind11 CONFIG REQUIRED)

# Use FetchContent to automatically download and build PMP library
include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)


FetchContent_Declare(
    pmp
    GIT_REPOSITORY https://github.com/pmp-library/pmp-library.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

# Set PMP options before making it available
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "Build PMP examples")
set(PMP_BUILD_TESTS OFF CACHE BOOL "Build PMP tests")
set(PMP_BUILD_DOCS OFF CACHE BOOL "Build PMP documentation")
set(PMP_BUILD_VIS OFF CACHE BOOL "Build PMP visualization")

# Make PMP available
FetchContent_MakeAvailable(pmp)

# Debug: Print PMP paths
message(STATUS "PMP source directory: ${pmp_SOURCE_DIR}")
message(STATUS "PMP binary directory: ${pmp_BINARY_DIR}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Python_INCLUDE_DIRS}
    ../../../include
)

# Wait for PMP to be available before accessing its properties
FetchContent_GetProperties(pmp)
if(NOT pmp_POPULATED)
    FetchContent_Populate(pmp)
endif()

# Find source files (support both .cpp and .cxx extensions)
pybind11_add_module(${MODULE_NAME}
    binding.cxx
)

# Link against PMP library
# target_link_libraries(pmppy PRIVATE pmp)

# Include directories - make sure to get them after PMP is populated
target_include_directories(pmppy PRIVATE 
    ${pmp_SOURCE_DIR}/src
    ${pmp_BINARY_DIR}/src  # For generated headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Debug: Print include directories
get_target_property(PMPPY_INCLUDES pmppy INCLUDE_DIRECTORIES)
message(STATUS "pmppy include directories: ${PMPPY_INCLUDES}")

# Make sure PMP is built before pmppy
add_dependencies(pmppy pmp)

# Compiler-specific options
target_compile_definitions(pmppy PRIVATE 
    VERSION_INFO="${PROJECT_VERSION}"
    PMP_SCALAR_TYPE=32  # Use float instead of double for better Python integration
)

# Platform-specific compiler flags
if(MSVC)
    target_compile_options(pmppy PRIVATE 
        /W4 
        /bigobj  # Handle large object files
    )
    # Disable specific MSVC warnings that are common with pybind11
    target_compile_definitions(pmppy PRIVATE 
        _SCL_SECURE_NO_WARNINGS 
        _CRT_SECURE_NO_WARNINGS
    )
else()
    target_compile_options(pmppy PRIVATE 
        -Wall 
        -Wextra 
        -Wno-unused-parameter  # pybind11 generates unused parameters
        -fvisibility=hidden    # Better symbol management
    )
endif()

# Enable optimizations for Release builds
target_compile_options(pmppy PRIVATE 
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# Optional: Enable OpenMP if available
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    target_link_libraries(pmppy PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(pmppy PRIVATE PMP_HAVE_OPENMP)
    message(STATUS "OpenMP found and enabled for parallel processing")
else()
    message(STATUS "OpenMP not found - parallel processing disabled")
endif()

# Optional: Enable Eigen if available (PMP can use it for some algorithms)
find_package(Eigen3 QUIET)
if(Eigen3_FOUND)
    target_link_libraries(pmppy PRIVATE Eigen3::Eigen)
    target_compile_definitions(pmppy PRIVATE PMP_HAVE_EIGEN)
    message(STATUS "Eigen3 found and enabled")
else()
    message(STATUS "Eigen3 not found - some advanced algorithms may be unavailable")
endif()

# Set module properties
set_target_properties(pmppy PROPERTIES
    PREFIX ""  # Remove 'lib' prefix on Unix
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Ensure proper RPATH handling on macOS and Linux
if(APPLE)
    set_target_properties(pmppy PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH ON
    )
elseif(UNIX)
    set_target_properties(pmppy PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH ON
    )
endif()

# Print build information
message(STATUS "=== pmppy Build Configuration ===")
message(STATUS "Python version: ${Python_VERSION}")
message(STATUS "Python executable: ${Python_EXECUTABLE}")
message(STATUS "pybind11 version: ${pybind11_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# Optional: Install rules for development
if(SKBUILD)
    # When building with scikit-build
    install(TARGETS pmppy DESTINATION .)
else()
    # For development builds, copy to source directory
    add_custom_command(TARGET pmppy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pmppy> ${CMAKE_SOURCE_DIR}/
        COMMENT "Copying pmppy module to source directory for development"
    )
endif()

# Optional: Create a target for running tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example.py")
    add_custom_target(run_example
        COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/example.py
        DEPENDS pmppy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running pmppy example script"
    )
endif()

# Add a custom target for cleaning Python cache
add_custom_target(clean_python
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/*.pyc
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/pmppy*.so
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/pmppy*.pyd
    COMMENT "Cleaning Python cache and compiled modules"
)