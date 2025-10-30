cmake_minimum_required(VERSION 4.0)
project(basic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(WIN32)
    # Option 1: Set Python path explicitly (uncomment and adjust path)
    # set(Python_EXECUTABLE "C:/Python313/")
    
    # # Option 2: Use py launcher to find Python
    execute_process(
        COMMAND py -3 -c "import sys; print(sys.executable)"
        OUTPUT_VARIABLE PYTHON_FROM_PY
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)

include_directories(../../../include)

# Create the Python module
pybind11_add_module(basic binding.cxx)
