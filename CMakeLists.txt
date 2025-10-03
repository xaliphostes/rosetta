cmake_minimum_required(VERSION 3.17)
project(rosetta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(include)

add_subdirectory(examples/cpp/simple)
add_subdirectory(examples/cpp/game)
