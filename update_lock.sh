#!/usr/bin/env bash
set -euo pipefail

# Usage: ./update_lock.sh [profile]
# Generates/updates a Conan lockfile (conan.lock) for reproducible builds.
# Optional first argument: profile name (must exist in ~/.conan2/profiles). Defaults to 'default'.

PROFILE=${1:-default}

if ! command -v conan >/dev/null 2>&1; then
  echo "conan executable not found in PATH" >&2
  exit 1
fi

# Ensure build folder exists for layout-generated files
BUILD_DIR=build
mkdir -p "$BUILD_DIR"

# Create or update lockfile
# We pass same settings autodetected earlier by CMake invocation if available; keep it simple here.

echo "Generating lockfile with profile: $PROFILE" >&2
conan lock create . \
  --lockfile-out=conan.lock \
  --profile=$PROFILE \
  --build=missing

echo "Lockfile written to conan.lock" >&2
