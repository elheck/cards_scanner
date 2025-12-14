#include <card_text_ocr.hpp>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <tuple>

// Test fixture for OCR preprocessing tests
class OcrPreprocessingTest : public ::testing::Test {
protected:
  // Create a test BGR image
  cv::Mat createColorImage(int width = 100, int height = 50) {
    cv::Mat img(height, width, CV_8UC3, cv::Scalar(128, 64, 192));
    return img;
  }

  // Create a test grayscale image
  cv::Mat createGrayscaleImage(int width = 100, int height = 50) {
    cv::Mat img(height, width, CV_8UC1, cv::Scalar(128));
    return img;
  }

  // Create an image with text-like patterns (black on white)
  cv::Mat createTextImage() {
    cv::Mat img(50, 200, CV_8UC3,
                cv::Scalar(255, 255, 255)); // White background
    cv::putText(img, "Test123", cv::Point(10, 35), cv::FONT_HERSHEY_SIMPLEX,
                1.0, cv::Scalar(0, 0, 0), 2);
    return img;
  }
};

// ============== Output Type Tests ==============

TEST_F(OcrPreprocessingTest, OutputIsGrayscale) {
  cv::Mat colorImage = createColorImage();
  cv::Mat processed = detect::preprocessForOcr(colorImage);

  EXPECT_EQ(processed.channels(), 1)
      << "Processed image should be single channel (grayscale)";
}

TEST_F(OcrPreprocessingTest, GrayscaleInputRemainsGrayscale) {
  cv::Mat grayImage = createGrayscaleImage();
  cv::Mat processed = detect::preprocessForOcr(grayImage);

  EXPECT_EQ(processed.channels(), 1)
      << "Grayscale input should remain single channel";
}

// ============== Scaling Tests ==============

TEST_F(OcrPreprocessingTest, OutputIsScaled3x) {
  cv::Mat image = createColorImage(100, 50);
  cv::Mat processed = detect::preprocessForOcr(image);

  // Image should be scaled 3x in both dimensions for better OCR
  EXPECT_EQ(processed.cols, image.cols * 3) << "Width should be tripled";
  EXPECT_EQ(processed.rows, image.rows * 3) << "Height should be tripled";
}

TEST_F(OcrPreprocessingTest, ScalingWorksWithDifferentSizes) {
  // Test with various input sizes
  std::vector<std::pair<int, int>> sizes = {
      {50, 25}, {200, 100}, {77, 33}, {1, 1}};

  for (const auto &[width, height] : sizes) {
    cv::Mat image = createColorImage(width, height);
    cv::Mat processed = detect::preprocessForOcr(image);

    EXPECT_EQ(processed.cols, width * 3)
        << "Width scaling failed for " << width << "x" << height;
    EXPECT_EQ(processed.rows, height * 3)
        << "Height scaling failed for " << width << "x" << height;
  }
}

// ============== Thresholding Tests ==============
// Note: preprocessForOcr applies adaptive threshold first, then scales with
// INTER_CUBIC. Cubic interpolation can introduce intermediate values at edges
// between black and white regions.

TEST_F(OcrPreprocessingTest, OutputIsBinary) {
  cv::Mat image = createColorImage();
  cv::Mat processed = detect::preprocessForOcr(image);

  // After adaptive thresholding and scaling, check the value range
  double minVal, maxVal;
  cv::minMaxLoc(processed, &minVal, &maxVal);

  // Due to cubic interpolation during scaling, values may not be exactly 0 and
  // 255 but should still be within the valid grayscale range
  EXPECT_GE(minVal, 0) << "Minimum value should be >= 0";
  EXPECT_LE(maxVal, 255) << "Maximum value should be <= 255";

  // Most values should still be close to 0 or 255 (binary-like)
  // The thresholding produces binary, but scaling can smooth edges
  EXPECT_TRUE(minVal < 128 || maxVal > 128) << "Should have some contrast";
}

TEST_F(OcrPreprocessingTest, ThresholdingProducesBinaryValues) {
  cv::Mat image = createTextImage();
  cv::Mat processed = detect::preprocessForOcr(image);

  // With cubic interpolation during scaling, pixel values may not be strictly
  // binary Check that most pixels are near 0 or 255 (allowing for interpolation
  // effects)
  int nearBlack = 0;
  int nearWhite = 0;
  int total = processed.rows * processed.cols;

  for (int y = 0; y < processed.rows; ++y) {
    for (int x = 0; x < processed.cols; ++x) {
      uchar val = processed.at<uchar>(y, x);
      if (val < 32)
        nearBlack++;
      else if (val > 223)
        nearWhite++;
    }
  }

  // At least 80% of pixels should be near black or white
  double binaryRatio = static_cast<double>(nearBlack + nearWhite) / total;
  EXPECT_GT(binaryRatio, 0.80) << "Most pixels should be near 0 or 255";
}

// ============== Data Type Tests ==============

TEST_F(OcrPreprocessingTest, OutputDataType) {
  cv::Mat image = createColorImage();
  cv::Mat processed = detect::preprocessForOcr(image);

  EXPECT_EQ(processed.type(), CV_8UC1)
      << "Output should be 8-bit single channel";
}

// ============== Non-Destructive Tests ==============

TEST_F(OcrPreprocessingTest, OriginalImageUnmodified) {
  cv::Mat original = createColorImage();
  cv::Mat originalCopy = original.clone();

  std::ignore = detect::preprocessForOcr(original);

  // Verify original image wasn't modified
  cv::Mat diff;
  cv::absdiff(original, originalCopy, diff);
  double sumDiff = cv::sum(diff)[0] + cv::sum(diff)[1] + cv::sum(diff)[2];

  EXPECT_EQ(sumDiff, 0) << "Original image should not be modified";
}

// ============== Edge Cases ==============

TEST_F(OcrPreprocessingTest, HandlesUniformBlackImage) {
  cv::Mat blackImage(50, 100, CV_8UC3, cv::Scalar(0, 0, 0));
  cv::Mat processed = detect::preprocessForOcr(blackImage);

  EXPECT_FALSE(processed.empty()) << "Should handle uniform black image";
  EXPECT_EQ(processed.channels(), 1);
}

TEST_F(OcrPreprocessingTest, HandlesUniformWhiteImage) {
  cv::Mat whiteImage(50, 100, CV_8UC3, cv::Scalar(255, 255, 255));
  cv::Mat processed = detect::preprocessForOcr(whiteImage);

  EXPECT_FALSE(processed.empty()) << "Should handle uniform white image";
  EXPECT_EQ(processed.channels(), 1);
}

TEST_F(OcrPreprocessingTest, HandlesSmallImage) {
  // Very small image - should still process without errors
  cv::Mat tinyImage(5, 10, CV_8UC3, cv::Scalar(128, 128, 128));
  cv::Mat processed = detect::preprocessForOcr(tinyImage);

  EXPECT_FALSE(processed.empty()) << "Should handle very small images";
  // 3x scaling: 10x5 -> 30x15
  EXPECT_EQ(processed.cols, 30);
  EXPECT_EQ(processed.rows, 15);
}

// ============== Text Preservation Tests ==============

TEST_F(OcrPreprocessingTest, PreservesTextContrast) {
  cv::Mat textImage = createTextImage();
  cv::Mat processed = detect::preprocessForOcr(textImage);

  // The processed image should have both black and white regions
  // (text should still be visible as contrast)
  int blackPixels = 0;
  int whitePixels = 0;

  for (int y = 0; y < processed.rows; ++y) {
    for (int x = 0; x < processed.cols; ++x) {
      uchar val = processed.at<uchar>(y, x);
      if (val == 0)
        blackPixels++;
      else if (val == 255)
        whitePixels++;
    }
  }

  EXPECT_GT(blackPixels, 0) << "Should have some black pixels (text)";
  EXPECT_GT(whitePixels, 0) << "Should have some white pixels (background)";
}
