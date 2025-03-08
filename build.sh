#!/bin/bash

# Define the build directory
BUILD_DIR="build"

# Initialize verbose flag to false
VERBOSE=false

# Parse command line arguments
while getopts "v" opt; do
  case $opt in
    v)
      VERBOSE=true
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

# Change to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project if CMakeCache.txt doesn't exist
if [ ! -f "CMakeCache.txt" ]; then
    echo "Configuring the project with CMake..."
    cmake ..
fi

# Build the project with or without verbose output
echo "Building the project..."
if [ "$VERBOSE" = true ]; then
    cmake --build . --verbose
else
    cmake --build .
fi

# Return to the original directory
cd ..

echo "Build process completed."
