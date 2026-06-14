#!/bin/bash

# Build and run System Monitor unit tests
# This script configures CMake, builds the tests, and executes them

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build_tests"

echo "System Monitor Unit Test Build Script"
echo "====================================="
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed. Please install CMake 3.14 or later."
    echo "Ubuntu/Debian: sudo apt-get install cmake"
    echo "macOS: brew install cmake"
    exit 1
fi

# Check for C compiler
if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
    echo "ERROR: No C compiler found. Please install GCC or Clang."
    echo "Ubuntu/Debian: sudo apt-get install build-essential"
    exit 1
fi

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake configuration..."
cmake "$PROJECT_ROOT"

echo ""
echo "Building test executables..."
cmake --build . --target test_statistics_store test_cpu_monitor

echo ""
echo "====================================="
echo "Tests built successfully!"
echo "====================================="
echo ""
echo "Running tests..."
echo ""

# Run tests with verbose output
ctest -V --output-on-failure

echo ""
echo "====================================="
echo "Test run complete!"
echo "====================================="
