#include "card_processor.hpp"
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
  CardProcessor processor;
  // Process all images in the sample_cards directory
  for (const auto &entry :
       std::filesystem::directory_iterator(std::string(SAMPLE_DATA_FOLDER))) {
    processor.loadImage(std::string(entry.path()));
    EXPECT_TRUE(processor.processCard());
    processor.displayImages();
  }
  EXPECT_TRUE(true);
}
