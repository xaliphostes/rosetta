cmake_minimum_required(VERSION 4.0)
project(rosetta)

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
    
    # if(PYTHON_FROM_PY)
    #     set(Python_EXECUTABLE ${PYTHON_FROM_PY})
    #     message(STATUS "Found Python via py launcher: ${Python_EXECUTABLE}")
    # endif()
    
    # # Avoid Windows Store Python stub
    # set(Python_FIND_REGISTRY LAST)
    # set(Python_FIND_STRATEGY LOCATION)
    
    # # Prefer 64-bit Python on 64-bit Windows
    # if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    #     set(Python_FIND_ARCH 64)
    # else()
    #     set(Python_FIND_ARCH 32)
    # endif()
endif()

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(pybind11)
# add_subdirectory(pybind11)

include_directories(../../../include)

## Find Python and pybind11
#find_package(Python COMPONENTS Interpreter Development REQUIRED)
#find_package(pybind11 REQUIRED)

# Create the Python module
pybind11_add_module(rosetta binding.cxx)
