cmake_minimum_required(VERSION 3.18)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
endif()

project(card_scanner 
    VERSION 1.0.0
    LANGUAGES CXX 
    DESCRIPTION "MTG Card Scanner with OpenCV"
)

# Define test samples directory relative to project root
set(TEST_SAMPLES_FOLDER "${CMAKE_SOURCE_DIR}/tests/test_samples")

# Add compile definitions for paths
add_compile_definitions(
    SAMPLE_DATA_FOLDER="${CMAKE_SOURCE_DIR}/tests/sample_cards"
    TEST_SAMPLES_FOLDER="${TEST_SAMPLES_FOLDER}"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing at the top level
enable_testing()

# Conan configuration for version 2.x
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

# Use root conanfile.txt (and optional lockfile) instead of inline REQUIRES list.
# Prefer architecture-specific lockfile (conan-<arch>.lock) if available.
set(_ARCH_HINT "")
if(CMAKE_SYSTEM_PROCESSOR)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _ARCH_HINT)
endif()
if(DEFINED CMAKE_HOST_SYSTEM_PROCESSOR AND NOT _ARCH_HINT)
    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _ARCH_HINT)
endif()

set(_CANDIDATE_LOCKFILES)
if(_ARCH_HINT MATCHES "arm64|aarch64")
    list(APPEND _CANDIDATE_LOCKFILES "${CMAKE_SOURCE_DIR}/conan-armv8.lock")
endif()
if(_ARCH_HINT MATCHES "x86_64|amd64")
    list(APPEND _CANDIDATE_LOCKFILES "${CMAKE_SOURCE_DIR}/conan-x86_64.lock")
endif()

foreach(lockfile IN LISTS _CANDIDATE_LOCKFILES)
    if(NOT CONAN_LOCKFILE AND EXISTS "${lockfile}")
        set(CONAN_LOCKFILE "${lockfile}")
    endif()
endforeach()

if(CONAN_LOCKFILE)
    message(STATUS "Using Conan lockfile: ${CONAN_LOCKFILE}")
else()
    message(WARNING "No Conan lockfile found (expected one of: ${_CANDIDATE_LOCKFILES}). Proceeding unlocked.")
endif()

# Install dependencies
conan_cmake_autodetect(settings)

set(_conan_install_args
    PATH_OR_REFERENCE "${CMAKE_SOURCE_DIR}"  # root with conanfile.txt
    BUILD missing
    SETTINGS ${settings}
)
if(CONAN_LOCKFILE)
    list(APPEND _conan_install_args LOCKFILE "${CONAN_LOCKFILE}")
endif()
conan_cmake_install(${_conan_install_args})

# Add the directory where Conan places the CMake configuration files to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")
set(_CONAN_GEN_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators")
if(EXISTS "${_CONAN_GEN_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${_CONAN_GEN_DIR}")
elseif(EXISTS "${CMAKE_BINARY_DIR}/generators")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/generators")
endif()


# Add source directories
add_subdirectory(src)
# Add the test subdirectories
add_subdirectory(tests/integration)
add_subdirectory(tests/unit)
