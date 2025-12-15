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

# Download Tesseract trained data if not present
# Using tessdata_best for better accuracy (larger file, slower but more accurate)
set(TESSDATA_DIR "${CMAKE_BINARY_DIR}/tessdata")
set(TESSDATA_LANG "eng")
set(TESSDATA_FILE "${TESSDATA_DIR}/${TESSDATA_LANG}.traineddata")

# Check if file exists and has non-zero size
if(EXISTS "${TESSDATA_FILE}")
    file(SIZE "${TESSDATA_FILE}" TESSDATA_SIZE)
    if(TESSDATA_SIZE EQUAL 0)
        message(WARNING "Tessdata file exists but is empty, removing and re-downloading...")
        file(REMOVE "${TESSDATA_FILE}")
    endif()
endif()

if(NOT EXISTS "${TESSDATA_FILE}")
    message(STATUS "Downloading Tesseract trained data (${TESSDATA_LANG}) - best quality...")
    file(MAKE_DIRECTORY "${TESSDATA_DIR}")
    
    # Try multiple mirrors with retry logic
    set(DOWNLOAD_URLS
        "https://github.com/tesseract-ocr/tessdata_best/raw/main/${TESSDATA_LANG}.traineddata"
        "https://raw.githubusercontent.com/tesseract-ocr/tessdata_best/main/${TESSDATA_LANG}.traineddata"
    )
    
    set(DOWNLOAD_SUCCESS FALSE)
    foreach(URL ${DOWNLOAD_URLS})
        message(STATUS "Attempting download from: ${URL}")
        file(DOWNLOAD
            "${URL}"
            "${TESSDATA_FILE}"
            TIMEOUT 120
            TLS_VERIFY ON
            STATUS DOWNLOAD_STATUS
        )
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
        if(DOWNLOAD_RESULT EQUAL 0)
            # Verify the file was actually downloaded
            file(SIZE "${TESSDATA_FILE}" TESSDATA_SIZE)
            if(TESSDATA_SIZE GREATER 0)
                set(DOWNLOAD_SUCCESS TRUE)
                message(STATUS "Successfully downloaded ${TESSDATA_LANG}.traineddata (${TESSDATA_SIZE} bytes)")
                break()
            else()
                message(WARNING "Download succeeded but file is empty, trying next URL...")
                file(REMOVE "${TESSDATA_FILE}")
            endif()
        else()
            list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR)
            message(WARNING "Download failed: ${DOWNLOAD_ERROR}")
        endif()
    endforeach()
    
    if(NOT DOWNLOAD_SUCCESS)
        message(FATAL_ERROR "Failed to download Tesseract trained data from all available sources. Please check your internet connection or download manually from: https://github.com/tesseract-ocr/tessdata_best/blob/main/${TESSDATA_LANG}.traineddata")
    endif()
else()
    file(SIZE "${TESSDATA_FILE}" TESSDATA_SIZE)
    message(STATUS "Tesseract trained data already present: ${TESSDATA_FILE} (${TESSDATA_SIZE} bytes)")
endif()

# Set TESSDATA_PREFIX for runtime
set(TESSDATA_PREFIX "${TESSDATA_DIR}" CACHE PATH "Path to Tesseract trained data")
add_compile_definitions(TESSDATA_PREFIX="${TESSDATA_PREFIX}")

# Add source directories
add_subdirectory(src)
# Add the test subdirectories
add_subdirectory(tests/integration)
add_subdirectory(tests/unit)
