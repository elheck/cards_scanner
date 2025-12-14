#include <card_detector.hpp>
#include <cmath>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <vector>

// Test fixture for sortCorners tests
class SortCornersTest : public ::testing::Test {
protected:
  // Helper to check if two points are approximately equal
  bool pointsEqual(const cv::Point2f &a, const cv::Point2f &b,
                   float epsilon = 0.001f) {
    return std::abs(a.x - b.x) < epsilon && std::abs(a.y - b.y) < epsilon;
  }

  // Verify corners are in order: TL, TR, BR, BL
  void verifyCornerOrder(const std::vector<cv::Point2f> &sorted,
                         const cv::Point2f &expectedTL,
                         const cv::Point2f &expectedTR,
                         const cv::Point2f &expectedBR,
                         const cv::Point2f &expectedBL) {
    ASSERT_EQ(sorted.size(), 4u);
    EXPECT_TRUE(pointsEqual(sorted[0], expectedTL))
        << "Top-left mismatch: expected (" << expectedTL.x << ","
        << expectedTL.y << ") got (" << sorted[0].x << "," << sorted[0].y
        << ")";
    EXPECT_TRUE(pointsEqual(sorted[1], expectedTR))
        << "Top-right mismatch: expected (" << expectedTR.x << ","
        << expectedTR.y << ") got (" << sorted[1].x << "," << sorted[1].y
        << ")";
    EXPECT_TRUE(pointsEqual(sorted[2], expectedBR))
        << "Bottom-right mismatch: expected (" << expectedBR.x << ","
        << expectedBR.y << ") got (" << sorted[2].x << "," << sorted[2].y
        << ")";
    EXPECT_TRUE(pointsEqual(sorted[3], expectedBL))
        << "Bottom-left mismatch: expected (" << expectedBL.x << ","
        << expectedBL.y << ") got (" << sorted[3].x << "," << sorted[3].y
        << ")";
  }
};

// Test with corners already in correct order (TL, TR, BR, BL)
TEST_F(SortCornersTest, CornersAlreadyInOrder) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(0, 0),     // TL
      cv::Point2f(100, 0),   // TR
      cv::Point2f(100, 150), // BR
      cv::Point2f(0, 150)    // BL
  };

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(0, 0), cv::Point2f(100, 0),
                    cv::Point2f(100, 150), cv::Point2f(0, 150));
}

// Test with corners in reverse order (BL, BR, TR, TL)
TEST_F(SortCornersTest, CornersInReverseOrder) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(0, 150),   // BL
      cv::Point2f(100, 150), // BR
      cv::Point2f(100, 0),   // TR
      cv::Point2f(0, 0)      // TL
  };

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(0, 0), cv::Point2f(100, 0),
                    cv::Point2f(100, 150), cv::Point2f(0, 150));
}

// Test with corners in random order
TEST_F(SortCornersTest, CornersInRandomOrder) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(100, 0),  // TR
      cv::Point2f(0, 150),  // BL
      cv::Point2f(0, 0),    // TL
      cv::Point2f(100, 150) // BR
  };

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(0, 0), cv::Point2f(100, 0),
                    cv::Point2f(100, 150), cv::Point2f(0, 150));
}

// Test with non-axis-aligned rectangle (tilted card)
TEST_F(SortCornersTest, TiltedRectangle) {
  // A rectangle tilted 10 degrees
  std::vector<cv::Point2f> corners = {
      cv::Point2f(20, 10),  // TL (shifted right and down a bit)
      cv::Point2f(110, 30), // TR
      cv::Point2f(90, 160), // BR
      cv::Point2f(0, 140)   // BL
  };

  auto sorted = detect::detail::sortCorners(corners);
  // After sorting, the top two (by y) should be first
  verifyCornerOrder(sorted, cv::Point2f(20, 10), // TL
                    cv::Point2f(110, 30),        // TR
                    cv::Point2f(90, 160),        // BR
                    cv::Point2f(0, 140));        // BL
}

// Test with square (equal width and height)
TEST_F(SortCornersTest, SquareCorners) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(50, 50),   // TL
      cv::Point2f(150, 50),  // TR
      cv::Point2f(150, 150), // BR
      cv::Point2f(50, 150)   // BL
  };

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(50, 50), cv::Point2f(150, 50),
                    cv::Point2f(150, 150), cv::Point2f(50, 150));
}

// Test with floating point precision
TEST_F(SortCornersTest, FloatingPointPrecision) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(0.5f, 0.5f), cv::Point2f(99.5f, 0.5f),
      cv::Point2f(99.5f, 149.5f), cv::Point2f(0.5f, 149.5f)};

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(0.5f, 0.5f), cv::Point2f(99.5f, 0.5f),
                    cv::Point2f(99.5f, 149.5f), cv::Point2f(0.5f, 149.5f));
}

// Test with larger coordinates (simulating high-res image)
TEST_F(SortCornersTest, LargeCoordinates) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(1000, 2000), // Mixed order
      cv::Point2f(5000, 500), cv::Point2f(500, 500), cv::Point2f(5000, 2000)};

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(
      sorted, cv::Point2f(500, 500), // TL
      cv::Point2f(5000, 500),        // TR
      cv::Point2f(5000, 2000),       // BR
      cv::Point2f(1000, 2000));      // BL - note: 1000 < 5000, so this is BL
}

// Test parallelogram (non-rectangular quadrilateral)
TEST_F(SortCornersTest, Parallelogram) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(50, 0),    // TL (shifted)
      cv::Point2f(150, 0),   // TR
      cv::Point2f(100, 100), // BR (shifted left)
      cv::Point2f(0, 100)    // BL
  };

  auto sorted = detect::detail::sortCorners(corners);
  verifyCornerOrder(sorted, cv::Point2f(50, 0), cv::Point2f(150, 0),
                    cv::Point2f(100, 100), cv::Point2f(0, 100));
}
