#include <detection_builder.hpp>
#include <pic_helper.hpp>
#include <path_helper.hpp>

#include <gtest/gtest.h>
#include <filesystem>
#include <opencv2/opencv.hpp>

class DetectionBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure we have the test data directory
        ASSERT_TRUE(std::filesystem::exists(misc::getSamplesPath()))
            << "Test data directory not found";
    }

    bool hasNameRegion(const cv::Mat& image) {
        // Convert to HSV to detect green rectangle (name region)
        cv::Mat hsv;
        cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

        // Green color range in HSV
        cv::Scalar lower_green(55, 50, 50);
        cv::Scalar upper_green(85, 255, 255);

        // Create mask for green color
        cv::Mat mask;
        cv::inRange(hsv, lower_green, upper_green, mask);

        // Count non-zero pixels (green pixels)
        int green_pixels = cv::countNonZero(mask);
        return green_pixels > 0;
    }

    workflow::DetectionBuilder builder{workflow::CardType::modernNormal};
};

// Test that processes each sample card and verifies the output
TEST_F(DetectionBuilderTest, ProcessAllSampleCards) {
    for (const auto& entry : std::filesystem::directory_iterator(misc::getSamplesPath())) {
        // Process the card
        cv::Mat processed_card = builder.process(entry.path());
        
        // Basic sanity checks
        ASSERT_FALSE(processed_card.empty()) 
            << "Failed to process card from " << entry.path();
        ASSERT_EQ(processed_card.channels(), 3) 
            << "Processed card should have 3 channels (BGR)";
        
        // Verify that name region was detected (should have green rectangle)
        EXPECT_TRUE(hasNameRegion(processed_card))
            << "Name region not detected in card from " << entry.path();
        
        // Save the processed image for manual verification
        auto output_folder = misc::getTestSamplesPath() / "detection_builder";
        EXPECT_TRUE(misc::saveImage(output_folder, processed_card))
            << "Failed to save processed card from " << entry.path();
    }
}

// Test error handling for invalid input
TEST_F(DetectionBuilderTest, HandleInvalidInput) {
    auto nonexistent_file = misc::getSamplesPath() / "nonexistent.jpg";
    EXPECT_THROW(builder.process(nonexistent_file), std::runtime_error);
}

// Test with different card types (currently only modernNormal is supported)
TEST_F(DetectionBuilderTest, UnsupportedCardType) {
    workflow::DetectionBuilder unsupported_builder(static_cast<workflow::CardType>(99));
    auto sample_file = *std::filesystem::directory_iterator(misc::getSamplesPath());
    EXPECT_THROW(unsupported_builder.process(sample_file.path()), std::runtime_error);
}