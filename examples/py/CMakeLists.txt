cmake_minimum_required(VERSION 3.15)
project(RosettaTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add all subdirectories that have CMakeLists.txt
file(GLOB SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(ALL_TARGETS "")

foreach(SUBDIR ${SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR} AND 
       EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
        add_subdirectory(${SUBDIR})
    endif()
endforeach()

# =============================================================================
# Custom target to run all Python tests
# =============================================================================
add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "========================================"
    COMMAND ${CMAKE_COMMAND} -E echo "Running Python Tests"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================"
    COMMENT "Running all Python test scripts"
)

# Find all test_*.py files in the build directory
file(GLOB TEST_FILES "${CMAKE_BINARY_DIR}/test_*.py")

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    add_custom_command(TARGET run POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Running: ${TEST_NAME}"
        COMMAND python3.14 ${TEST_FILE} || ${CMAKE_COMMAND} -E true
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running ${TEST_NAME}"
    )
endforeach()

add_custom_command(TARGET run POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "========================================"
    COMMAND ${CMAKE_COMMAND} -E echo "All tests complete!"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================"
)