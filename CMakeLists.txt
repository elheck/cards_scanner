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
        opencv/4.10.0
        gtest/1.13.0
    OPTIONS 
        opencv/*:with_cuda=False 
        opencv/*:with_wayland=False
        opencv/*:with_gtk=True
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

# Find the Conan-installed packages
find_package(OpenCV CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Define the core library
add_library(card_processor
    src/card_processor.cpp
)

# Include directories for the core library
target_include_directories(card_processor PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries for the core library
target_link_libraries(card_processor PUBLIC
    opencv::opencv
)

# Add executable
add_executable(card_scanner
    src/main.cpp
)

# Link the executable with the core library
target_link_libraries(card_scanner PRIVATE
    card_processor
)

# Install targets
install(TARGETS card_scanner
    RUNTIME DESTINATION bin
)

# Add the test subdirectory
add_subdirectory(tests/integration)
