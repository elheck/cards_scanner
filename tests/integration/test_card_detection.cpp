#include <detection/card_detector.hpp>
#include <detection/tilt_corrector.hpp>
#include <misc/pic_helper.hpp>
#include <misc/path_helper.hpp>

#include <filesystem>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <string>


class CardDetectionTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Ensure we have the test data directory
    ASSERT_TRUE(std::filesystem::exists(misc::getSamplesPath()))
        << "Test data directory not found";
  }
};

// Integration test combining detection and text reading
TEST_F(CardDetectionTest, EndToEndCardProcessing) {
  // Process all images in the sample_cards directory
  for (const auto &entry :
       std::filesystem::directory_iterator(misc::getSamplesPath())) {
    auto pic = detect::processCards(entry.path());
    EXPECT_FALSE(pic.empty()) << "Failed to process card from " << entry.path();
    
    auto folder = std::filesystem::path(entry.path().parent_path().parent_path() / "tests" / "detection");
    EXPECT_TRUE(misc::saveImage(folder, pic));
  }
}

// Integration test for card detection and tilt correction
TEST_F(CardDetectionTest, CardDetectionAndTiltCorrection) {
  // Process all images in the sample_cards directory
  for (const auto &entry :
       std::filesystem::directory_iterator(misc::getSamplesPath())) {
    auto pic = detect::processCards(entry.path());
    EXPECT_FALSE(pic.empty()) << "Failed to process card from " << entry.path();
    
    // Correct tilt
    pic = detect::correctCardTilt(pic);
    EXPECT_FALSE(pic.empty()) << "Failed to correct tilt for " << entry.path();
    
    // Save the processed image
    auto folder = std::filesystem::path(entry.path().parent_path().parent_path() / "tests" / "tilt_correction");
    EXPECT_TRUE(misc::saveImage(folder, pic));
  }
}