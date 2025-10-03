# examples/scripting/lua/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(luarosetta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch Sol3
include(FetchContent)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)

# Find Lua (Sol3 will use this)
find_package(Lua REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    ${LUA_INCLUDE_DIR}
)

# Create executable
add_executable(${PROJECT_NAME} binding.cxx)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    sol2::sol2
    ${LUA_LIBRARIES}
)

# Copy test script to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test.lua
    ${CMAKE_CURRENT_BINARY_DIR}/test.lua
    COPYONLY
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()

message(STATUS "Lua bindings configured")
message(STATUS "Lua version: ${LUA_VERSION_STRING}")
message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libraries: ${LUA_LIBRARIES}")