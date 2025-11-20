cmake_minimum_required(VERSION 3.8)
project(em)

set(MODULE_NAME "allem")

add_definitions(-std=c++20)
set (CMAKE_CXX_STANDARD 20)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Python_INCLUDE_DIRS}
    ../../../include
    ../third
)
add_library(${MODULE_NAME} SHARED binding.cxx)
add_executable(${MODULE_NAME}js binding.cxx)

set_target_properties(${MODULE_NAME}js PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "--bind -s MODULARIZE=1 -s EXPORT_NAME='InheritanceModule' -s SINGLE_FILE=1 -s NO_DISABLE_EXCEPTION_CATCHING"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.mjs)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test.mjs
        ${CMAKE_CURRENT_SOURCE_DIR}/build/test.mjs
        COPYONLY
    )
endif()
