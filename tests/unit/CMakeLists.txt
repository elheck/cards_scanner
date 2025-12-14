cmake_minimum_required(VERSION 3.18)

# Find required packages if not already found
if(NOT TARGET GTest::gtest)
    find_package(GTest REQUIRED)
endif()
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

# Define the unit test executable
add_executable(test_unit
    test_sort_corners.cpp
    test_region_extraction.cpp
    test_ocr_preprocessing.cpp
    test_load_image.cpp
    test_scryfall_client.cpp
)

# Include directories for the test
target_include_directories(test_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/src/detection/include
    ${CMAKE_SOURCE_DIR}/src/misc/include
    ${CMAKE_SOURCE_DIR}/src/api/include
    ${OpenCV_INCLUDE_DIRS}
)

# Link the test with required libraries
target_link_libraries(test_unit PRIVATE
    card_processor_lib
    misc_lib
    api_lib
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(test_unit)
