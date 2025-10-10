cmake_minimum_required(VERSION 3.15)
project(rosetta_functions)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# ============================================================================
# CRITICAL: Avoid Windows Store Python stub
# ============================================================================
if(WIN32)
    # Explicitly exclude Windows Store Python paths
    set(Python3_FIND_REGISTRY NEVER)  # Don't use registry
    set(Python3_FIND_STRATEGY LOCATION)  # Search in standard locations
    
    # Try multiple methods to find real Python
    
    # Method 1: Try py launcher first (most reliable on Windows)
    execute_process(
        COMMAND py -3 -c "import sys; print(sys.executable)"
        OUTPUT_VARIABLE PYTHON_FROM_PY
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE PY_RESULT
    )
    
    if(PY_RESULT EQUAL 0 AND PYTHON_FROM_PY)
        # Verify it's not the WindowsApps stub
        string(FIND "${PYTHON_FROM_PY}" "WindowsApps" WINDOWSAPPS_POS)
        if(WINDOWSAPPS_POS EQUAL -1)
            set(Python3_EXECUTABLE ${PYTHON_FROM_PY})
            message(STATUS "Found Python via py launcher: ${Python3_EXECUTABLE}")
        endif()
    endif()
    
    # Method 2: Check common installation paths
    if(NOT Python3_EXECUTABLE)
        set(PYTHON_SEARCH_PATHS
            "C:/Python313"
            "C:/Python312"
            "C:/Python311"
            "C:/Python310"
            "C:/Program Files/Python313"
            "C:/Program Files/Python312"
            "C:/Program Files/Python311"
            "C:/Program Files/Python310"
            "$ENV{LOCALAPPDATA}/Programs/Python/Python313"
            "$ENV{LOCALAPPDATA}/Programs/Python/Python312"
            "$ENV{LOCALAPPDATA}/Programs/Python/Python311"
            "$ENV{LOCALAPPDATA}/Programs/Python/Python310"
        )
        
        foreach(SEARCH_PATH ${PYTHON_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/python.exe")
                # Verify it's not a stub
                execute_process(
                    COMMAND "${SEARCH_PATH}/python.exe" -c "import sys; print(sys.executable)"
                    OUTPUT_VARIABLE TEST_PYTHON
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                    RESULT_VARIABLE TEST_RESULT
                )
                
                if(TEST_RESULT EQUAL 0)
                    string(FIND "${TEST_PYTHON}" "WindowsApps" WINDOWSAPPS_POS)
                    if(WINDOWSAPPS_POS EQUAL -1)
                        set(Python3_EXECUTABLE "${SEARCH_PATH}/python.exe")
                        message(STATUS "Found Python at: ${Python3_EXECUTABLE}")
                        break()
                    endif()
                endif()
            endif()
        endforeach()
    endif()
    
    # Method 3: Let user specify
    if(NOT Python3_EXECUTABLE)
        message(WARNING "Could not automatically find Python. Please specify:")
        message(WARNING "  cmake -DPython3_EXECUTABLE=C:/Python313/python.exe ..")
    endif()
endif()

# Find Python3 with our hints
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Verify we didn't get the WindowsApps stub
string(FIND "${Python3_EXECUTABLE}" "WindowsApps" WINDOWSAPPS_CHECK)
if(NOT WINDOWSAPPS_CHECK EQUAL -1)
    message(FATAL_ERROR 
        "CMake found Windows Store Python stub, not real Python!\n"
        "Please specify your real Python installation:\n"
        "  cmake -DPython3_EXECUTABLE=C:/Python313/python.exe ..\n"
        "Or disable Windows Store Python:\n"
        "  Settings → Apps → Apps & Features → App Execution Aliases\n"
        "  Disable 'python.exe' and 'python3.exe'"
    )
endif()

message(STATUS "==========================================")
message(STATUS "Python Configuration:")
message(STATUS "  Executable: ${Python3_EXECUTABLE}")
message(STATUS "  Version: ${Python3_VERSION}")
message(STATUS "  Include: ${Python3_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${Python3_LIBRARIES}")
message(STATUS "==========================================")

# Fetch pybind11
include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# Include Rosetta headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include)

# Create the Python module
pybind11_add_module(rosetta_functions binding.cxx)

# Copy test script to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test.py
    ${CMAKE_CURRENT_BINARY_DIR}/test.py
    COPYONLY
)

message(STATUS "")
message(STATUS "Build configured successfully!")
message(STATUS "To build: cmake --build . --config Release")
message(STATUS "To test:  ${Python3_EXECUTABLE} test.py")
message(STATUS "")