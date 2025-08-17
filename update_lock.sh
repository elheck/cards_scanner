#!/usr/bin/env bash
set -euo pipefail

# Usage: ./update_lock.sh
# Generates/updates architecture-specific Conan lockfiles for both x86_64 and armv8
# architectures: conan-x86_64.lock and conan-armv8.lock for reproducible builds.

if ! command -v conan >/dev/null 2>&1; then
  echo "conan executable not found in PATH" >&2
  exit 1
fi

# Ensure build folder exists for layout-generated files
BUILD_DIR=build
mkdir -p "$BUILD_DIR"

# Function to generate lockfile for a specific architecture
generate_lockfile() {
  local profile="$1"
  local arch="$2"
  
  echo "Generating lockfile for arch: ${arch} -> conan-${arch}.lock" >&2
  conan lock create . \
    --lockfile-out="conan-${arch}.lock" \
    --profile="$profile" \
    --build=missing \
    -s arch="${arch}"

  echo "Lockfile written to conan-${arch}.lock" >&2
}

# Generate lockfiles for both architectures
echo "Updating lockfiles for both architectures..." >&2

# Check if x86_64 profile exists, otherwise use default with arch override
if conan profile path x86_64 >/dev/null 2>&1; then
  generate_lockfile "x86_64" "x86_64"
else
  generate_lockfile "default" "x86_64"
fi

# Generate armv8 lockfile using default profile with arch override
generate_lockfile "default" "armv8"

echo "Both lockfiles updated successfully!" >&2
