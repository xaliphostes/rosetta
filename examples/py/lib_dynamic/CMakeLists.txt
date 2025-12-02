cmake_minimum_required(VERSION 3.15)
project(NestedProject)

# Set C++ standard if needed
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get all subdirectories
file(GLOB SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

# Separate py-folders from non-py folders
set(PY_SUBDIRS "")
set(OTHER_SUBDIRS "")

foreach(SUBDIR ${SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR})
        # Check if subdirectory has CMakeLists.txt
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
            # Check if it starts with "py"
            string(SUBSTRING ${SUBDIR} 0 2 PREFIX)
            if(PREFIX STREQUAL "py")
                list(APPEND PY_SUBDIRS ${SUBDIR})
            else()
                list(APPEND OTHER_SUBDIRS ${SUBDIR})
            endif()
        endif()
    endif()
endforeach()

# Add non-py subdirectories first
message(STATUS "Building non-py subdirectories first:")
foreach(SUBDIR ${OTHER_SUBDIRS})
    message(STATUS "  - ${SUBDIR}")
    add_subdirectory(${SUBDIR})
endforeach()

# Add py subdirectories last (they may depend on the others)
message(STATUS "Building py subdirectories:")
foreach(SUBDIR ${PY_SUBDIRS})
    message(STATUS "  - ${SUBDIR}")
    add_subdirectory(${SUBDIR})
endforeach()