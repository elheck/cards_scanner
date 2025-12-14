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

# Remove the existing build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create a new build directory
echo "Creating build directory..."
mkdir "$BUILD_DIR" && cd "$BUILD_DIR"

# Run CMake to configure the project with compile_commands.json for clang-tidy
echo "Configuring the project with CMake..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

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