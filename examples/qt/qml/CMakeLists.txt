cmake_minimum_required(VERSION 3.16)
project(rosetta_qml_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================================
# Find Qt6 - with macOS auto-detection
# ============================================================================

# Auto-detect Qt installation on macOS
if(APPLE AND EXISTS "$ENV{HOME}/Qt")
    file(GLOB QT_VERSIONS "$ENV{HOME}/Qt/6.*")
    if(QT_VERSIONS)
        list(SORT QT_VERSIONS)
        list(GET QT_VERSIONS -1 QT_LATEST)
        message(STATUS "Found Qt at: ${QT_LATEST}/macos")
        set(CMAKE_PREFIX_PATH "${QT_LATEST}/macos" ${CMAKE_PREFIX_PATH})
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

# ============================================================================
# Rosetta Configuration
# ============================================================================

# Set path to Rosetta headers - adjust as needed
set(ROSETTA_INCLUDE_DIR "$ENV{HOME}/devs/c++/rosetta/include" 
    CACHE PATH "Path to Rosetta include directory")

if(NOT EXISTS "${ROSETTA_INCLUDE_DIR}/rosetta/rosetta.h")
    message(WARNING "Rosetta headers not found at ${ROSETTA_INCLUDE_DIR}")
    message(WARNING "Set ROSETTA_INCLUDE_DIR to your Rosetta include path")
elseif()
    message(STATUS "Found Rosetta headers ${ROSETTA_INCLUDE_DIR}")
endif()

# ============================================================================
# Application
# ============================================================================

qt_add_executable(rosetta_qml_demo
    main.cxx
    ${ROSETTA_INCLUDE_DIR}/rosetta/extensions/gui/qt/qml_bridge.h
)

target_include_directories(rosetta_qml_demo PRIVATE
    ${ROSETTA_INCLUDE_DIR}
)

qt_add_qml_module(rosetta_qml_demo
    URI RosettaQmlDemo
    VERSION 1.0
    # RESOURCE_PREFIX /qt/qml
    QML_FILES 
        Main.qml
        PropertyRow.qml
    # NO_RESOURCE_TARGET_PATH
)

target_link_libraries(rosetta_qml_demo PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
)

# ============================================================================
# Install
# ============================================================================

install(TARGETS rosetta_qml_demo
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
