cmake_minimum_required(VERSION 3.15)
project(rosetta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Node.js and npm
find_program(NODE_EXECUTABLE NAMES node nodejs)
find_program(NPM_EXECUTABLE NAMES npm)

if(NOT NODE_EXECUTABLE)
    message(FATAL_ERROR "Node.js not found. Please install Node.js")
endif()

if(NOT NPM_EXECUTABLE)
    message(FATAL_ERROR "npm not found. Please install npm")
endif()

# Get Node.js version
execute_process(
    COMMAND ${NODE_EXECUTABLE} --version
    OUTPUT_VARIABLE NODE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Found Node.js: ${NODE_EXECUTABLE} (version ${NODE_VERSION})")
message(STATUS "Found npm: ${NPM_EXECUTABLE}")

# Include directories
include_directories(../../../include)

# Copy files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package.json 
               ${CMAKE_CURRENT_BINARY_DIR}/package.json COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/binding.gyp 
               ${CMAKE_CURRENT_BINARY_DIR}/binding.gyp COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/binding.cxx 
               ${CMAKE_CURRENT_BINARY_DIR}/binding.cxx COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo.h 
               ${CMAKE_CURRENT_BINARY_DIR}/demo.h COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.js 
               ${CMAKE_CURRENT_BINARY_DIR}/test.js COPYONLY)

# Custom target for npm install
add_custom_target(npm_install
    COMMAND ${NPM_EXECUTABLE} install
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running npm install..."
)

# Custom target for building the addon
add_custom_target(build_addon
    COMMAND ${NPM_EXECUTABLE} run build
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS npm_install
    COMMENT "Building Node.js addon..."
)

# Custom target for testing
add_custom_target(test_js_bindings
    COMMAND ${NPM_EXECUTABLE} test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS build_addon
    COMMENT "Testing JavaScript bindings..."
)

# Add to main target
add_custom_target(js_bindings ALL
    DEPENDS build_addon
    COMMENT "JavaScript bindings target"
)

# Install targets
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/package.json
    ${CMAKE_CURRENT_BINARY_DIR}/binding.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/binding.gyp
    ${CMAKE_CURRENT_BINARY_DIR}/demo.h
    #${CMAKE_CURRENT_BINARY_DIR}/JavascriptBindingGenerator.h
    ${CMAKE_CURRENT_BINARY_DIR}/test.js
    DESTINATION js_bindings/
)

# Install built addon if it exists
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/
    DESTINATION js_bindings/build/
    OPTIONAL
)

message(STATUS "JavaScript bindings will be built in: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "To build manually:")
message(STATUS "  cd ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "  npm install")
message(STATUS "  npm run build")
message(STATUS "  npm test")