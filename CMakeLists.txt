cmake_minimum_required(VERSION 3.15)
project(OCR_Project 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Optical Character Recognition Project"
)

# Global configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type handling
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Conan integration
include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)

# Include project utilities
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Sanitizers)

# Project components
add_subdirectory(src)

# Installation targets
install(DIRECTORY include/ DESTINATION include)
install(TARGETS mtg_ocr DESTINATION bin)

# Package configuration
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OCRProjectConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OCRProjectConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OCRProject
)
