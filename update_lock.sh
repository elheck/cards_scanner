#!/usr/bin/env bash
set -euo pipefail

# Usage: ./update_lock.sh [profile]
# Generates/updates an architecture-specific Conan lockfile: conan-<arch>.lock
# (e.g. conan-x86_64.lock, conan-armv8.lock) for reproducible builds across hosts.
# Optional first argument: profile name (must exist in ~/.conan2/profiles). Defaults to 'default'.

PROFILE=${1:-default}

if ! command -v conan >/dev/null 2>&1; then
  echo "conan executable not found in PATH" >&2
  exit 1
fi

# Ensure build folder exists for layout-generated files
BUILD_DIR=build
mkdir -p "$BUILD_DIR"

echo "Detecting profile ($PROFILE) to determine architecture..." >&2
conan profile detect --force >/dev/null 2>&1 || true

PROFILE_PATH="$(conan profile path "$PROFILE" 2>/dev/null || true)"
if [ -z "$PROFILE_PATH" ] || [ ! -f "$PROFILE_PATH" ]; then
  echo "Profile '$PROFILE' not found (looked for $PROFILE_PATH)" >&2
  exit 1
fi

ARCH=$(grep -E '^arch=' "$PROFILE_PATH" | head -n1 | cut -d'=' -f2 | tr -d '[:space:]')
if [ -z "$ARCH" ]; then
  echo "Could not determine arch from profile ($PROFILE_PATH)" >&2
  exit 1
fi

LOCKFILE="conan-${ARCH}.lock"
echo "Generating lockfile for arch: ${ARCH} -> ${LOCKFILE}" >&2
conan lock create . \
  --lockfile-out="${LOCKFILE}" \
  --profile="$PROFILE" \
  --build=missing

echo "Lockfile written to ${LOCKFILE}" >&2
