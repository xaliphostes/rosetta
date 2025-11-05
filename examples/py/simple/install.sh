#!/bin/bash
# Automated installation script for Rosetta Python bindings
# Supports Ubuntu/Debian and macOS

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored messages
info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -f /etc/debian_version ]; then
            OS="debian"
        elif [ -f /etc/redhat-release ]; then
            OS="redhat"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    else
        error "Unsupported OS: $OSTYPE"
    fi
    info "Detected OS: $OS"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install dependencies
install_dependencies() {
    info "Installing dependencies..."
    
    if [ "$OS" = "debian" ]; then
        info "Using apt package manager"
        sudo apt update
        sudo apt install -y build-essential cmake git python3-dev python3-pip
        
    elif [ "$OS" = "redhat" ]; then
        info "Using yum package manager"
        sudo yum groupinstall -y "Development Tools"
        sudo yum install -y cmake git python3-devel python3-pip
        
    elif [ "$OS" = "macos" ]; then
        info "Using Homebrew"
        
        # Check if Homebrew is installed
        if ! command_exists brew; then
            warning "Homebrew not found. Installing..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        fi
        
        brew install cmake python
    fi
    
    success "Dependencies installed"
}

# Install Python packages
install_python_packages() {
    info "Installing Python packages..."
    
    # Upgrade pip
    python3 -m pip install --upgrade pip
    
    # Install pybind11
    python3 -m pip install pybind11
    
    success "Python packages installed"
}

# Check requirements
check_requirements() {
    info "Checking requirements..."
    
    local missing_deps=()
    
    # Check C++ compiler
    if ! command_exists g++ && ! command_exists clang++; then
        missing_deps+=("C++ compiler (g++ or clang++)")
    else
        if command_exists g++; then
            local gcc_version=$(g++ -dumpversion | cut -d. -f1)
            if [ "$gcc_version" -lt 10 ]; then
                warning "GCC version $gcc_version detected. GCC 10+ recommended for full C++20 support"
            fi
        fi
    fi
    
    # Check CMake
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    else
        local cmake_version=$(cmake --version | head -n1 | grep -oE '[0-9]+\.[0-9]+')
        local cmake_major=$(echo $cmake_version | cut -d. -f1)
        if [ "$cmake_major" -lt 3 ]; then
            missing_deps+=("cmake 3.15+ (current: $cmake_version)")
        fi
    fi
    
    # Check Python
    if ! command_exists python3; then
        missing_deps+=("python3")
    else
        local python_version=$(python3 --version | grep -oE '[0-9]+\.[0-9]+')
        info "Python version: $python_version"
    fi
    
    # Check pip
    if ! command_exists pip3 && ! python3 -m pip --version >/dev/null 2>&1; then
        missing_deps+=("pip3")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        error "Missing dependencies: ${missing_deps[*]}"
    fi
    
    success "All requirements met"
}

# Build using CMake
build_cmake() {
    info "Building with CMake..."
    
    # Create build directory
    if [ -d "build" ]; then
        warning "Build directory exists. Cleaning..."
        rm -rf build
    fi
    
    mkdir build
    cd build
    
    # Configure
    info "Configuring..."
    cmake .. || error "CMake configuration failed"
    
    # Build
    info "Building (this may take a minute)..."
    local num_cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    make -j${num_cores} || error "Build failed"
    
    cd ..
    success "Build completed"
}

# Build using setup.py
build_setuppy() {
    info "Building with setup.py..."
    
    python3 setup.py build || error "Build failed"
    
    success "Build completed"
}

# Run tests
run_tests() {
    info "Running tests..."
    
    if [ -f "build/test_bindings.py" ]; then
        cd build
        python3 ../test_bindings.py || error "Tests failed"
        cd ..
    elif [ -f "test_bindings.py" ]; then
        python3 test_bindings.py || error "Tests failed"
    else
        warning "Test file not found, skipping tests"
        return
    fi
    
    success "All tests passed"
}

# Install
install_package() {
    info "Installing package..."
    
    local install_method=$1
    
    if [ "$install_method" = "system" ]; then
        if [ -d "build" ]; then
            cd build
            sudo make install || error "Installation failed"
            cd ..
        else
            error "Build directory not found. Build first."
        fi
    elif [ "$install_method" = "user" ]; then
        python3 -m pip install --user -e . || error "Installation failed"
    else
        python3 -m pip install -e . || error "Installation failed"
    fi
    
    success "Package installed"
}

# Print usage
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Automated installation script for Rosetta Python bindings

OPTIONS:
    -h, --help              Show this help message
    -d, --deps-only         Install dependencies only
    -b, --build-only        Build only (skip dependency installation)
    -t, --test-only         Run tests only (requires existing build)
    -i, --install METHOD    Install after building (METHOD: system, user, dev)
    --skip-tests            Skip running tests
    --use-setuppy           Use setup.py instead of CMake

EXAMPLES:
    $0                      # Full installation (deps + build + test)
    $0 --deps-only          # Install dependencies only
    $0 --build-only         # Build without installing dependencies
    $0 -i user              # Build and install for current user
    $0 --skip-tests         # Build without running tests

EOF
}

# Main function
main() {
    local deps_only=false
    local build_only=false
    local test_only=false
    local skip_tests=false
    local use_setuppy=false
    local install_method=""
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -d|--deps-only)
                deps_only=true
                shift
                ;;
            -b|--build-only)
                build_only=true
                shift
                ;;
            -t|--test-only)
                test_only=true
                shift
                ;;
            --skip-tests)
                skip_tests=true
                shift
                ;;
            --use-setuppy)
                use_setuppy=true
                shift
                ;;
            -i|--install)
                install_method="$2"
                shift 2
                ;;
            *)
                error "Unknown option: $1. Use --help for usage."
                ;;
        esac
    done
    
    echo "======================================"
    echo "  Rosetta Python Bindings Installer  "
    echo "======================================"
    echo ""
    
    # Detect OS
    detect_os
    
    # Handle different modes
    if [ "$test_only" = true ]; then
        info "Test-only mode"
        run_tests
        exit 0
    fi
    
    if [ "$deps_only" = true ]; then
        info "Dependencies-only mode"
        install_dependencies
        install_python_packages
        success "Dependencies installed successfully!"
        exit 0
    fi
    
    # Install dependencies (unless build-only mode)
    if [ "$build_only" = false ]; then
        install_dependencies
        install_python_packages
    fi
    
    # Check requirements
    check_requirements
    
    # Build
    if [ "$use_setuppy" = true ]; then
        build_setuppy
    else
        build_cmake
    fi
    
    # Run tests (unless skipped)
    if [ "$skip_tests" = false ]; then
        run_tests
    fi
    
    # Install (if requested)
    if [ -n "$install_method" ]; then
        install_package "$install_method"
    fi
    
    # Print success message
    echo ""
    echo "======================================"
    success "Installation completed successfully!"
    echo "======================================"
    echo ""
    
    if [ "$use_setuppy" = true ]; then
        info "To test, run: python3 test_bindings.py"
    else
        info "To test, run: cd build && python3 ../test_bindings.py"
    fi
    
    if [ -z "$install_method" ]; then
        info "To install system-wide, run: sudo make install (from build directory)"
        info "Or run this script with: $0 -i system"
    fi
}

# Run main function
main "$@"