#include <detection/card_detector.hpp>
#include <misc/pic_helper.hpp>

#include <filesystem>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <string>


class CardDetectionTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Ensure we have the test data directory
    ASSERT_TRUE(std::filesystem::exists(std::string(SAMPLE_DATA_FOLDER)))
        << "Test data directory not found";
  }
};

// Integration test combining detection and text reading
TEST_F(CardDetectionTest, EndToEndCardProcessing) {
  // Create a card processor
  detect::CardDetector processor;
  // Process all images in the sample_cards directory
  for (const auto &entry :
       std::filesystem::directory_iterator(std::string(SAMPLE_DATA_FOLDER))) {
    EXPECT_TRUE(processor.loadImage(std::string(entry.path())));
    auto pic = processor.processCards();
    auto folder = std::filesystem::path(entry.path().parent_path() / "sample_out");
    EXPECT_TRUE(cs::saveImage(folder, pic));
  }
  EXPECT_TRUE(true);
}
