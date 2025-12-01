cmake_minimum_required(VERSION 3.10)

project(FlowerLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# Create SHARED (dynamic) library
# =============================================================================
add_library(flower SHARED
    src/flower.cxx
)

# Define export macro for Windows DLL
target_compile_definitions(flower PRIVATE FLOWER_EXPORTS)

target_include_directories(flower
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set output directory to ../lib
set_target_properties(flower PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"  # .lib on Windows
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"  # .so/.dylib
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"  # .dll on Windows
)

# Set version properties for the shared library
set_target_properties(flower PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# On Unix, set visibility to hidden by default (recommended for shared libs)
if(NOT WIN32)
    set_target_properties(flower PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

# Create the lib directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib")

# =============================================================================
# Installation
# =============================================================================
install(TARGETS flower
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/flower.h
    DESTINATION include
)