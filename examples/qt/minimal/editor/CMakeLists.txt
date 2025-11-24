cmake_minimum_required(VERSION 3.16)
project(objectinspector_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(ROSETTA_INCLUDE_DIR "$ENV{HOME}/devs/c++/rosetta/include" 
    CACHE PATH "Path to Rosetta include directory")

if(NOT EXISTS "${ROSETTA_INCLUDE_DIR}/rosetta/rosetta.h")
    message(WARNING "Rosetta headers not found at ${ROSETTA_INCLUDE_DIR}")
    message(WARNING "Set ROSETTA_INCLUDE_DIR to your Rosetta include path")
elseif()
    message(STATUS "Found Rosetta headers ${ROSETTA_INCLUDE_DIR}")
endif()

add_executable(${PROJECT_NAME}
    main.cxx
    ${ROSETTA_INCLUDE_DIR}/rosetta/extensions/qt/qt_property_editor.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROSETTA_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
)