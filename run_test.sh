#!/bin/bash

# Define the build directory
BUILD_DIR="build"

# Change to the build directory
cd "$BUILD_DIR"

# From your build directory
cmake ..

cmake --build .

ctest -V
