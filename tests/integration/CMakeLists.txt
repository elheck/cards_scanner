# Find required packages if not already found
if(NOT TARGET GTest::gtest)
    find_package(GTest REQUIRED)
endif()

# Define the integration test executable
add_executable(test_card_detection
    test_card_detection.cpp
)

target_compile_definitions(test_card_detection PUBLIC "SAMPLE_DATA_FOLDER=\"${CMAKE_SOURCE_DIR}/tests/sample_cards\"")

# Include directories for the test
target_include_directories(test_card_detection PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link the test with the card_processor library and testing framework
target_link_libraries(test_card_detection PRIVATE
    card_processor
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_card_detection)