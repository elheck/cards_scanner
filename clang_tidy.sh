#!/bin/bash

# Define the build directory
BUILD_DIR="build"

# Create or clean the build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure CMake with compile commands database
echo "Configuring CMake with compile commands database..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Go back to root
cd ..

# Find all C++ source and header files
echo "Running clang-tidy on all C++ files..."
find src \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 | xargs -0 clang-tidy -p="$BUILD_DIR"