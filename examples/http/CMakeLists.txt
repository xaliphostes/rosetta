project(http)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.26.0  # Use latest stable version
)
FetchContent_MakeAvailable(httplib)

add_executable(${PROJECT_NAME} main.cxx)

# Link cpp-httplib
target_link_libraries(${PROJECT_NAME} PRIVATE httplib::httplib)

# For threading support (required by httplib)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 wsock32)
endif()