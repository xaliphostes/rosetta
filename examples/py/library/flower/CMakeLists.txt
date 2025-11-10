cmake_minimum_required(VERSION 3.10)

project(FlowerLib VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library
add_library(flower
    src/flower.cxx
)

# Specify include directories
target_include_directories(flower
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Optional: Create an executable for testing
option(BUILD_EXAMPLE "Build example executable" OFF)

if(BUILD_EXAMPLE)
    add_executable(flower_example example/main.cpp)
    target_link_libraries(flower_example PRIVATE flower)
endif()

# Installation rules
install(TARGETS flower
    EXPORT FlowerLibTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT FlowerLibTargets
    FILE FlowerLibTargets.cmake
    NAMESPACE Flower::
    DESTINATION lib/cmake/FlowerLib
)