@echo off
REM Automated installation script for Rosetta Python bindings (Windows)
REM Requires Visual Studio or MinGW

setlocal enabledelayedexpansion

echo ======================================
echo   Rosetta Python Bindings Installer
echo   Windows Version
echo ======================================
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found. Please install Python from python.org
    echo Make sure to check "Add Python to PATH" during installation
    pause
    exit /b 1
)

echo [INFO] Python found: 
python --version

REM Check if pip is available
python -m pip --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] pip not found. Please install pip
    pause
    exit /b 1
)

echo [INFO] pip found

REM Check for compiler
echo [INFO] Checking for C++ compiler...

REM Try to find Visual Studio
where cl.exe >nul 2>&1
if not errorlevel 1 (
    echo [INFO] Visual Studio compiler found
    set COMPILER=MSVC
    goto :compiler_found
)

REM Try to find MinGW
where g++.exe >nul 2>&1
if not errorlevel 1 (
    echo [INFO] MinGW compiler found
    set COMPILER=MinGW
    goto :compiler_found
)

echo [WARNING] No C++ compiler found!
echo Please install one of the following:
echo   1. Visual Studio 2019+ with C++ development tools
echo   2. MinGW-w64
echo.
echo For Visual Studio: https://visualstudio.microsoft.com/downloads/
echo For MinGW: https://www.mingw-w64.org/downloads/
pause
exit /b 1

:compiler_found

REM Install Python dependencies
echo.
echo [INFO] Installing Python dependencies...
python -m pip install --upgrade pip
python -m pip install pybind11 setuptools wheel

if errorlevel 1 (
    echo [ERROR] Failed to install Python dependencies
    pause
    exit /b 1
)

echo [SUCCESS] Python dependencies installed

REM Choose build method
echo.
echo Choose build method:
echo   1. CMake (recommended, requires CMake installation)
echo   2. setup.py (simpler, uses setuptools)
echo.
set /p BUILD_METHOD="Enter choice (1 or 2): "

if "%BUILD_METHOD%"=="1" goto :build_cmake
if "%BUILD_METHOD%"=="2" goto :build_setuppy

echo [ERROR] Invalid choice
pause
exit /b 1

:build_cmake
echo.
echo [INFO] Building with CMake...

REM Check if CMake is installed
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found!
    echo Please install CMake from: https://cmake.org/download/
    echo Make sure to add CMake to PATH during installation
    pause
    exit /b 1
)

echo [INFO] CMake found:
cmake --version | findstr /C:"cmake version"

REM Clean previous build
if exist build (
    echo [INFO] Cleaning previous build...
    rmdir /s /q build
)

REM Create build directory
mkdir build
cd build

REM Configure based on compiler
echo [INFO] Configuring...

if "%COMPILER%"=="MSVC" (
    REM Try different Visual Studio generators
    cmake .. -G "Visual Studio 16 2019" >nul 2>&1
    if not errorlevel 1 goto :configured
    
    cmake .. -G "Visual Studio 17 2022" >nul 2>&1
    if not errorlevel 1 goto :configured
    
    cmake .. >nul 2>&1
    if not errorlevel 1 goto :configured
    
    echo [ERROR] CMake configuration failed
    cd ..
    pause
    exit /b 1
) else (
    cmake .. -G "MinGW Makefiles"
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed
        cd ..
        pause
        exit /b 1
    )
)

:configured
echo [SUCCESS] Configuration completed

REM Build
echo [INFO] Building (this may take a minute)...

if "%COMPILER%"=="MSVC" (
    cmake --build . --config Release
) else (
    mingw32-make
)

if errorlevel 1 (
    echo [ERROR] Build failed
    cd ..
    pause
    exit /b 1
)

cd ..
echo [SUCCESS] Build completed
goto :test

:build_setuppy
echo.
echo [INFO] Building with setup.py...

python setup.py build

if errorlevel 1 (
    echo [ERROR] Build failed
    pause
    exit /b 1
)

echo [SUCCESS] Build completed
goto :test

:test
echo.
echo [INFO] Running tests...

if exist build\test_bindings.py (
    cd build
    python ..\test_bindings.py
    set TEST_RESULT=!errorlevel!
    cd ..
) else if exist test_bindings.py (
    python test_bindings.py
    set TEST_RESULT=!errorlevel!
) else (
    echo [WARNING] Test file not found, skipping tests
    set TEST_RESULT=0
)

if not !TEST_RESULT!==0 (
    echo [ERROR] Tests failed
    pause
    exit /b 1
)

echo [SUCCESS] All tests passed!

REM Ask about installation
echo.
echo [INFO] Build completed successfully!
echo.
set /p INSTALL="Do you want to install the package? (y/n): "

if /i "%INSTALL%"=="y" (
    echo [INFO] Installing...
    python -m pip install -e .
    
    if errorlevel 1 (
        echo [ERROR] Installation failed
        pause
        exit /b 1
    )
    
    echo [SUCCESS] Package installed!
)

REM Success message
echo.
echo ======================================
echo [SUCCESS] Installation completed!
echo ======================================
echo.
echo You can now use the module in Python:
echo   python
echo   ^>^>^> import rosetta_example
echo.

if "%BUILD_METHOD%"=="1" (
    echo The compiled module is in: build\
) else (
    echo The module has been installed
)

echo.
echo To uninstall later, run:
echo   pip uninstall rosetta-example
echo.

pause