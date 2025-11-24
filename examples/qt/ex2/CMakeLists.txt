cmake_minimum_required(VERSION 3.16)
project(Rosetta3DEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================================
# Qt6 Configuration
# ============================================================================
# Option 1: Set Qt6_DIR explicitly (recommended)
# Adjust the version number (6.7.0, 6.6.0, etc.) to match your installation
if(APPLE AND EXISTS "$ENV{HOME}/Qt")
    # Auto-detect Qt version in ~/Qt/
    file(GLOB QT_VERSIONS "$ENV{HOME}/Qt/6.*")
    if(QT_VERSIONS)
        list(SORT QT_VERSIONS)
        list(GET QT_VERSIONS -1 QT_LATEST)  # Get latest version
        set(CMAKE_PREFIX_PATH "${QT_LATEST}/macos" ${CMAKE_PREFIX_PATH})
        message(STATUS "Found Qt at: ${QT_LATEST}/macos")
    endif()
endif()

# Option 2: You can also pass it on command line:
#   cmake -DCMAKE_PREFIX_PATH=~/Qt/6.7.0/macos ..
#
# Option 3: Set environment variable before running cmake:
#   export CMAKE_PREFIX_PATH=~/Qt/6.7.0/macos

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Quick3D
    QuickControls2
)

message(STATUS "Qt6 found at: ${Qt6_DIR}")

# Set path to Rosetta headers - adjust as needed
set(ROSETTA_INCLUDE_DIR "$ENV{HOME}/devs/c++/rosetta/include" 
    CACHE PATH "Path to Rosetta include directory")

if(NOT EXISTS "${ROSETTA_INCLUDE_DIR}/rosetta/rosetta.h")
    message(WARNING "Rosetta headers not found at ${ROSETTA_INCLUDE_DIR}")
    message(WARNING "Set ROSETTA_INCLUDE_DIR to your Rosetta include path")
elseif()
    message(STATUS "Found Rosetta headers ${ROSETTA_INCLUDE_DIR}")
endif()

# QML module - this embeds QML files into the executable
qt_add_executable(rosetta_3d_editor
    main.cxx
)

qt_add_qml_module(rosetta_3d_editor
    URI Rosetta3DEditor
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        Main.qml
        PropertySection.qml
        ColorDialog.qml
    SOURCES
        scene_objects.h
        rosetta_registration.h
    NO_RESOURCE_TARGET_PATH
)

target_include_directories(rosetta_3d_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROSETTA_INCLUDE_DIR}
)

target_link_libraries(rosetta_3d_editor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Quick3D
    Qt6::QuickControls2
)

# Set properties for deployment
set_target_properties(rosetta_3d_editor PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Install
install(TARGETS rosetta_3d_editor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)