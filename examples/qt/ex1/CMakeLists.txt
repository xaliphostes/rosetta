cmake_minimum_required(VERSION 3.16)
project(RosettaQtPropertyEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Set path to Rosetta headers - adjust as needed
set(ROSETTA_INCLUDE_DIR "$ENV{HOME}/devs/c++/rosetta/include" 
    CACHE PATH "Path to Rosetta include directory")

if(NOT EXISTS "${ROSETTA_INCLUDE_DIR}/rosetta/rosetta.h")
    message(WARNING "Rosetta headers not found at ${ROSETTA_INCLUDE_DIR}")
    message(WARNING "Set ROSETTA_INCLUDE_DIR to your Rosetta include path")
elseif()
    message(STATUS "Found Rosetta headers ${ROSETTA_INCLUDE_DIR}")
endif()


# Property Editor Library
add_library(rosetta_qt_property_editor INTERFACE)
target_include_directories(rosetta_qt_property_editor INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROSETTA_INCLUDE_DIR}
)
target_link_libraries(rosetta_qt_property_editor INTERFACE
    Qt6::Core
    Qt6::Widgets
)

# Example Application
add_executable(property_editor_demo
    main.cxx
)

target_link_libraries(property_editor_demo PRIVATE
    rosetta_qt_property_editor
    Qt6::Core
    Qt6::Widgets
)

# Install targets
install(FILES
    qt_property_editor.h
    qt_property_editor.hxx
    DESTINATION include/rosetta/qt
)