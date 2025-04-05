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

# Configure Conan for version 2.x
conan_cmake_configure(
    REQUIRES 
        zstd/1.5.5#1f239731dc45147c7fc2f54bfbde73df
        gtest/1.13.0#b79e331e44bc2ce4fca3cf59c56387b6
        spdlog/1.15.1#92e99f07f134481bce4b70c1a41060e7
        libassert/2.1.4#de095beca7f09b01b613dd4e03574f88
        ms-gsl/4.1.0#e2af591b5e8c3a3f6a570cf9cbcd7a87
        leptonica/1.83.1#58142618f0f8f12e20c333c45871c1d2
        tesseract/5.5.0#cc1f274c2b9c226ef36863aaf26c68ea
    OPTIONS 
    GENERATORS 
        CMakeDeps
)

# Install dependencies
conan_cmake_autodetect(settings)

conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${settings}
)

# Add the directory where Conan places the CMake configuration files to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")


# Add source directories
add_subdirectory(src)
# Add the test subdirectory
add_subdirectory(tests/integration)
