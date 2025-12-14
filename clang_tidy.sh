#!/bin/bash

# Exit on any error
set -e

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

# Run clang-tidy on each file and fail if any issues are found
echo "Running clang-tidy on all C++ files..."
FAILED=0

find src \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 | while IFS= read -r -d '' file; do
    echo "Checking $file..."
    if ! clang-tidy -p="$BUILD_DIR" "$file"; then
        echo "clang-tidy failed for $file"
        FAILED=1
    fi
done

exit $FAILED