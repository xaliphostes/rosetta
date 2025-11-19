cmake_minimum_required(VERSION 3.8)
project(test)

add_definitions(-std=c++20)
set (CMAKE_CXX_STANDARD 20)

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1) # windows/msvc needs explicit "export all"

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Python_INCLUDE_DIRS}
    ../../../include
)
add_library(inheritance SHARED binding.cxx)

add_executable(inheritancejs binding.cxx)
set_target_properties(inheritancejs PROPERTIES
    SUFFIX ".js"
    LINK_FLAGS "--bind -s MODULARIZE=1 -s EXPORT_NAME='InheritanceModule' -s SINGLE_FILE=1"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_module.mjs)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test_module.mjs
        ${CMAKE_CURRENT_SOURCE_DIR}/build/test_module.mjs
        COPYONLY
    )
endif()
