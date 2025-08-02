cmake_minimum_required(VERSION 3.18)

# Find required packages if not already found
if(NOT TARGET GTest::gtest)
    find_package(GTest REQUIRED)
endif()
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

# Define the integration test executable
add_executable(test_card_detection
    test_card_detection.cpp
    test_detection_builder.cpp
)

# Include directories for the test
target_include_directories(test_card_detection PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${OpenCV_INCLUDE_DIRS}
)

# Link the test with required libraries
target_link_libraries(test_card_detection PRIVATE
    card_processor_lib
    workflow_lib
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_card_detection)
