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
        gtest/1.13.0
    OPTIONS 
    GENERATORS CMakeDeps
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
