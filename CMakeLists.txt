cmake_minimum_required(VERSION 3.17)
project(rosetta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(include)

set(BUILD_TESTING ON CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------
# Detect all testing files
# ---------------------------------------------------------------------
function (auto_discover_tests)
    file(GLOB_RECURSE BASE_TEST_FILES "unittests/*_test.cxx")
    foreach(TEST_FILE ${BASE_TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()    
endfunction()

if (BUILD_TESTING)
    enable_testing()
    auto_discover_tests()

    add_subdirectory(examples/cpp)
endif()
