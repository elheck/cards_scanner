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
if(EXISTS "${CMAKE_SOURCE_DIR}/conan.lock")
    message(STATUS "Using Conan lockfile: ${CMAKE_SOURCE_DIR}/conan.lock")
    set(CONAN_LOCKFILE "${CMAKE_SOURCE_DIR}/conan.lock")
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
# Add the test subdirectory
add_subdirectory(tests/integration)
