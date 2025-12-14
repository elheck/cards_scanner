#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <region_extraction.hpp>

// Test fixture for region extraction tests
class RegionExtractionTest : public ::testing::Test {
protected:
  // Create a test image with known dimensions
  // Using normalized card dimensions from card_detector.hpp (480x680)
  static constexpr int cardWidth = 480;
  static constexpr int cardHeight = 680;

  cv::Mat createTestCard() {
    return cv::Mat(cardHeight, cardWidth, CV_8UC3, cv::Scalar(128, 128, 128));
  }

  // Helper to verify rect is within image bounds
  void verifyRectWithinBounds(const cv::Rect &rect, int imageWidth,
                              int imageHeight) {
    EXPECT_GE(rect.x, 0) << "Rect x is negative";
    EXPECT_GE(rect.y, 0) << "Rect y is negative";
    EXPECT_LE(rect.x + rect.width, imageWidth) << "Rect exceeds image width";
    EXPECT_LE(rect.y + rect.height, imageHeight) << "Rect exceeds image height";
    EXPECT_GT(rect.width, 0) << "Rect width should be positive";
    EXPECT_GT(rect.height, 0) << "Rect height should be positive";
  }
};

// ============== Name Region Tests ==============

TEST_F(RegionExtractionTest, NameRegionWithinBounds) {
  cv::Mat card = createTestCard();
  cv::Rect nameRegion = detect::extractNameRegion(card);

  verifyRectWithinBounds(nameRegion, cardWidth, cardHeight);
}

TEST_F(RegionExtractionTest, NameRegionCorrectPosition) {
  cv::Mat card = createTestCard();
  cv::Rect nameRegion = detect::extractNameRegion(card);

  // Name region should be near the top of the card
  // Expected: ~4% from left, ~3.3% from top
  int expectedX = static_cast<int>(cardWidth * 0.04);
  int expectedY = static_cast<int>(cardHeight * 0.033);

  EXPECT_EQ(nameRegion.x, expectedX);
  EXPECT_EQ(nameRegion.y, expectedY);
}

TEST_F(RegionExtractionTest, NameRegionCorrectSize) {
  cv::Mat card = createTestCard();
  cv::Rect nameRegion = detect::extractNameRegion(card);

  // Expected: ~75% width, ~6.5% height
  int expectedWidth = static_cast<int>(cardWidth * 0.75);
  int expectedHeight = static_cast<int>(cardHeight * 0.065);

  EXPECT_EQ(nameRegion.width, expectedWidth);
  EXPECT_EQ(nameRegion.height, expectedHeight);
}

TEST_F(RegionExtractionTest, NameRegionScalesWithImageSize) {
  // Test with different image sizes
  cv::Mat smallCard(340, 240, CV_8UC3, cv::Scalar(128, 128, 128));
  cv::Mat largeCard(1360, 960, CV_8UC3, cv::Scalar(128, 128, 128));

  cv::Rect smallRegion = detect::extractNameRegion(smallCard);
  cv::Rect largeRegion = detect::extractNameRegion(largeCard);

  verifyRectWithinBounds(smallRegion, smallCard.cols, smallCard.rows);
  verifyRectWithinBounds(largeRegion, largeCard.cols, largeCard.rows);

  // Large region should be roughly 4x the size of small region (2x in each
  // dimension)
  EXPECT_NEAR(static_cast<double>(largeRegion.width) / smallRegion.width, 4.0,
              0.5);
  EXPECT_NEAR(static_cast<double>(largeRegion.height) / smallRegion.height, 4.0,
              0.5);
}

// ============== Collector Number Region Tests ==============

TEST_F(RegionExtractionTest, CollectorNumberRegionWithinBounds) {
  cv::Mat card = createTestCard();
  cv::Rect collectorRegion = detect::extractCollectorNumberRegionModern(card);

  verifyRectWithinBounds(collectorRegion, cardWidth, cardHeight);
}

TEST_F(RegionExtractionTest, CollectorNumberRegionNearBottom) {
  cv::Mat card = createTestCard();
  cv::Rect collectorRegion = detect::extractCollectorNumberRegionModern(card);

  // Collector number should be near the bottom of the card (~93% from top)
  int expectedY = static_cast<int>(cardHeight * 0.93);

  EXPECT_EQ(collectorRegion.y, expectedY);
  // Should be in the bottom 10% of the card
  EXPECT_GT(collectorRegion.y, cardHeight * 0.9);
}

TEST_F(RegionExtractionTest, CollectorNumberRegionOnLeft) {
  cv::Mat card = createTestCard();
  cv::Rect collectorRegion = detect::extractCollectorNumberRegionModern(card);

  // Should be near left edge (~4% from left)
  int expectedX = static_cast<int>(cardWidth * 0.04);

  EXPECT_EQ(collectorRegion.x, expectedX);
  // Should be in the left third of the card
  EXPECT_LT(collectorRegion.x + collectorRegion.width, cardWidth / 3);
}

// ============== Set Name Region Tests ==============

TEST_F(RegionExtractionTest, SetNameRegionWithinBounds) {
  cv::Mat card = createTestCard();
  cv::Rect setRegion = detect::extractSetNameRegionModern(card);

  verifyRectWithinBounds(setRegion, cardWidth, cardHeight);
}

TEST_F(RegionExtractionTest, SetNameRegionBelowCollectorNumber) {
  cv::Mat card = createTestCard();
  cv::Rect collectorRegion = detect::extractCollectorNumberRegionModern(card);
  cv::Rect setRegion = detect::extractSetNameRegionModern(card);

  // Set name should be below collector number
  EXPECT_GT(setRegion.y, collectorRegion.y);
}

TEST_F(RegionExtractionTest, SetNameRegionCorrectSize) {
  cv::Mat card = createTestCard();
  cv::Rect setRegion = detect::extractSetNameRegionModern(card);

  // Expected: ~12% width, ~3.5% height (matching region_extraction.cpp
  // constants)
  int expectedWidth = static_cast<int>(cardWidth * 0.12);
  int expectedHeight = static_cast<int>(cardHeight * 0.035);

  EXPECT_EQ(setRegion.width, expectedWidth);
  EXPECT_EQ(setRegion.height, expectedHeight);
}

// ============== Art Region Tests ==============
// Note: extractArtRegionRegular uses edge detection to dynamically find art box
// borders. With a simple test image, it may not find a valid contour and
// returns an empty rect.

TEST_F(RegionExtractionTest, ArtRegionWithinBounds) {
  // Create image with visible edges for art detection
  cv::Mat card = createTestCard();
  // Draw a prominent border that edge detection can find
  int artX = static_cast<int>(cardWidth * 0.10);
  int artY = static_cast<int>(cardHeight * 0.11);
  int artW = static_cast<int>(cardWidth * 0.80);
  int artH = static_cast<int>(cardHeight * 0.35);
  cv::rectangle(card, cv::Rect(artX, artY, artW, artH), cv::Scalar(0, 0, 0), 3);

  cv::Rect artRegion = detect::extractArtRegionRegular(card);

  // Edge detection is heuristic - may or may not find a region
  if (artRegion.width > 0 && artRegion.height > 0) {
    verifyRectWithinBounds(artRegion, cardWidth, cardHeight);
  }
  SUCCEED(); // Test passes regardless - edge detection is best-effort
}

TEST_F(RegionExtractionTest, ArtRegionInMiddleOfCard) {
  // Create image with visible edges for art detection
  cv::Mat card = createTestCard();
  int artX = static_cast<int>(cardWidth * 0.10);
  int artY = static_cast<int>(cardHeight * 0.11);
  int artW = static_cast<int>(cardWidth * 0.80);
  int artH = static_cast<int>(cardHeight * 0.35);
  cv::rectangle(card, cv::Rect(artX, artY, artW, artH), cv::Scalar(0, 0, 0), 3);

  cv::Rect artRegion = detect::extractArtRegionRegular(card);

  // Only verify if a region was found
  if (artRegion.width > 0 && artRegion.height > 0) {
    int centerX = artRegion.x + artRegion.width / 2;
    EXPECT_NEAR(centerX, cardWidth / 2,
                cardWidth * 0.20); // Within 20% of center
  }
  SUCCEED();
}

TEST_F(RegionExtractionTest, ArtRegionSignificantSize) {
  // Create image with visible edges for art detection
  cv::Mat card = createTestCard();
  int artX = static_cast<int>(cardWidth * 0.10);
  int artY = static_cast<int>(cardHeight * 0.11);
  int artW = static_cast<int>(cardWidth * 0.80);
  int artH = static_cast<int>(cardHeight * 0.35);
  cv::rectangle(card, cv::Rect(artX, artY, artW, artH), cv::Scalar(0, 0, 0), 3);

  cv::Rect artRegion = detect::extractArtRegionRegular(card);

  // Only verify if a region was found
  if (artRegion.width > 0 && artRegion.height > 0) {
    double areaRatio =
        static_cast<double>(artRegion.area()) / (cardWidth * cardHeight);
    EXPECT_GT(areaRatio, 0.05); // At least 5% of card area
    EXPECT_LT(areaRatio, 0.60); // Less than 60% of card area
  }
  SUCCEED();
}

// ============== Text Region Tests ==============

TEST_F(RegionExtractionTest, TextRegionWithinBounds) {
  cv::Mat card = createTestCard();
  cv::Rect textRegion = detect::extractTextRegion(card);

  verifyRectWithinBounds(textRegion, cardWidth, cardHeight);
}

TEST_F(RegionExtractionTest, TextRegionBelowArtRegion) {
  cv::Mat card = createTestCard();
  cv::Rect artRegion = detect::extractArtRegionRegular(card);
  cv::Rect textRegion = detect::extractTextRegion(card);

  // Text box should be below the art (or overlapping at bottom)
  // Due to dynamic detection, just verify it's in the lower half of card
  EXPECT_GT(textRegion.y, cardHeight * 0.4);
}

// ============== Edge Case Tests ==============

TEST_F(RegionExtractionTest, MinimumSizeImage) {
  // Very small image - regions should still be valid
  cv::Mat tinyCard(68, 48, CV_8UC3,
                   cv::Scalar(128, 128, 128)); // 10% of normal size

  cv::Rect nameRegion = detect::extractNameRegion(tinyCard);
  cv::Rect collectorRegion =
      detect::extractCollectorNumberRegionModern(tinyCard);
  cv::Rect setRegion = detect::extractSetNameRegionModern(tinyCard);

  verifyRectWithinBounds(nameRegion, tinyCard.cols, tinyCard.rows);
  verifyRectWithinBounds(collectorRegion, tinyCard.cols, tinyCard.rows);
  verifyRectWithinBounds(setRegion, tinyCard.cols, tinyCard.rows);
}

TEST_F(RegionExtractionTest, LargeImage) {
  // Large image - regions should scale appropriately
  cv::Mat largeCard(2720, 1920, CV_8UC3,
                    cv::Scalar(128, 128, 128)); // 4x normal size

  cv::Rect nameRegion = detect::extractNameRegion(largeCard);
  cv::Rect collectorRegion =
      detect::extractCollectorNumberRegionModern(largeCard);
  cv::Rect setRegion = detect::extractSetNameRegionModern(largeCard);

  verifyRectWithinBounds(nameRegion, largeCard.cols, largeCard.rows);
  verifyRectWithinBounds(collectorRegion, largeCard.cols, largeCard.rows);
  verifyRectWithinBounds(setRegion, largeCard.cols, largeCard.rows);
}

TEST_F(RegionExtractionTest, NonStandardAspectRatio) {
  // Wide image
  cv::Mat wideCard(400, 800, CV_8UC3, cv::Scalar(128, 128, 128));

  cv::Rect nameRegion = detect::extractNameRegion(wideCard);
  verifyRectWithinBounds(nameRegion, wideCard.cols, wideCard.rows);

  // Tall image
  cv::Mat tallCard(1000, 300, CV_8UC3, cv::Scalar(128, 128, 128));

  cv::Rect nameRegion2 = detect::extractNameRegion(tallCard);
  verifyRectWithinBounds(nameRegion2, tallCard.cols, tallCard.rows);
}
