#include <card_detector.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <path_helper.hpp>
#include <tuple>

// Test fixture for loadImage tests
class LoadImageTest : public ::testing::Test {
protected:
  std::filesystem::path tempDir;

  void SetUp() override {
    // Create a temporary directory for test files
    tempDir = std::filesystem::temp_directory_path() / "load_image_tests";
    std::filesystem::create_directories(tempDir);
  }

  void TearDown() override {
    // Clean up temporary files
    std::filesystem::remove_all(tempDir);
  }

  // Create a valid test image file
  std::filesystem::path
  createValidImage(const std::string &name = "valid.jpg") {
    cv::Mat img(100, 100, CV_8UC3, cv::Scalar(128, 128, 128));
    auto path = tempDir / name;
    cv::imwrite(path.string(), img);
    return path;
  }

  // Create a corrupt image file (invalid content)
  std::filesystem::path
  createCorruptImage(const std::string &name = "corrupt.jpg") {
    auto path = tempDir / name;
    std::ofstream file(path, std::ios::binary);
    file << "This is not a valid image file content!!!";
    file.close();
    return path;
  }

  // Create an empty file
  std::filesystem::path createEmptyFile(const std::string &name = "empty.jpg") {
    auto path = tempDir / name;
    std::ofstream file(path);
    file.close();
    return path;
  }

  // Create a text file with wrong extension
  std::filesystem::path
  createTextFileWithImageExtension(const std::string &name = "text.png") {
    auto path = tempDir / name;
    std::ofstream file(path);
    file << "This is just a text file pretending to be an image.";
    file.close();
    return path;
  }
};

// ============== Successful Load Tests ==============

TEST_F(LoadImageTest, LoadValidJpegImage) {
  auto imagePath = createValidImage("test.jpg");
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should successfully load valid JPEG image";
  EXPECT_FALSE(original.empty()) << "Original image should not be empty";
  EXPECT_FALSE(undistorted.empty()) << "Undistorted image should not be empty";
}

TEST_F(LoadImageTest, LoadValidPngImage) {
  auto imagePath = createValidImage("test.png");
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should successfully load valid PNG image";
  EXPECT_FALSE(original.empty());
}

TEST_F(LoadImageTest, LoadValidBmpImage) {
  auto imagePath = createValidImage("test.bmp");
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should successfully load valid BMP image";
  EXPECT_FALSE(original.empty());
}

TEST_F(LoadImageTest, UndistortedImageIsCloneOfOriginal) {
  auto imagePath = createValidImage();
  cv::Mat original, undistorted;

  std::ignore = detect::detail::loadImage(imagePath, original, undistorted);

  // Undistorted should be a copy of original (with same dimensions and content)
  EXPECT_EQ(original.size(), undistorted.size());
  EXPECT_EQ(original.type(), undistorted.type());

  // But they should be different memory locations (clone, not reference)
  EXPECT_NE(original.data, undistorted.data);
}

// ============== Non-Existent File Tests ==============

TEST_F(LoadImageTest, NonExistentFileReturnsFalse) {
  std::filesystem::path nonExistentPath = tempDir / "does_not_exist.jpg";
  cv::Mat original, undistorted;

  bool result =
      detect::detail::loadImage(nonExistentPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for non-existent file";
}

TEST_F(LoadImageTest, NonExistentDirectoryReturnsFalse) {
  std::filesystem::path badPath = "/nonexistent/directory/image.jpg";
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(badPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for non-existent directory";
}

TEST_F(LoadImageTest, EmptyPathReturnsFalse) {
  std::filesystem::path emptyPath;
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(emptyPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for empty path";
}

// ============== Corrupt File Tests ==============

TEST_F(LoadImageTest, CorruptImageReturnsFalse) {
  auto corruptPath = createCorruptImage();
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(corruptPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for corrupt image file";
}

TEST_F(LoadImageTest, EmptyFileReturnsFalse) {
  auto emptyPath = createEmptyFile();
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(emptyPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for empty file";
}

TEST_F(LoadImageTest, TextFileWithImageExtensionReturnsFalse) {
  auto textPath = createTextFileWithImageExtension();
  cv::Mat original, undistorted;

  bool result = detect::detail::loadImage(textPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for text file with image extension";
}

// ============== Unsupported Format Tests ==============

TEST_F(LoadImageTest, UnsupportedFormatReturnsFalse) {
  // Create a file with unsupported extension
  auto unsupportedPath = tempDir / "file.xyz";
  std::ofstream file(unsupportedPath);
  file << "random content";
  file.close();

  cv::Mat original, undistorted;

  bool result =
      detect::detail::loadImage(unsupportedPath, original, undistorted);

  EXPECT_FALSE(result) << "Should fail for unsupported format";
}

// ============== Image Properties After Load ==============

TEST_F(LoadImageTest, LoadedImageHasCorrectDimensions) {
  // Create image with known dimensions
  cv::Mat testImg(200, 300, CV_8UC3, cv::Scalar(100, 150, 200));
  auto imagePath = tempDir / "dimensions_test.png";
  cv::imwrite(imagePath.string(), testImg);

  cv::Mat original, undistorted;
  std::ignore = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_EQ(original.rows, 200);
  EXPECT_EQ(original.cols, 300);
  EXPECT_EQ(original.channels(), 3);
}

TEST_F(LoadImageTest, LoadedImagePreservesContent) {
  // Create image with known content
  cv::Mat testImg(50, 50, CV_8UC3, cv::Scalar(50, 100, 150));
  auto imagePath = tempDir / "content_test.png";
  cv::imwrite(imagePath.string(), testImg);

  cv::Mat original, undistorted;
  std::ignore = detect::detail::loadImage(imagePath, original, undistorted);

  // Check a pixel value (PNG should preserve exactly)
  cv::Vec3b pixel = original.at<cv::Vec3b>(25, 25);
  EXPECT_EQ(pixel[0], 50);  // Blue
  EXPECT_EQ(pixel[1], 100); // Green
  EXPECT_EQ(pixel[2], 150); // Red
}

// ============== Edge Cases ==============

TEST_F(LoadImageTest, VerySmallImage) {
  cv::Mat tinyImg(1, 1, CV_8UC3, cv::Scalar(255, 255, 255));
  auto imagePath = tempDir / "tiny.png";
  cv::imwrite(imagePath.string(), tinyImg);

  cv::Mat original, undistorted;
  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should handle 1x1 image";
  EXPECT_EQ(original.rows, 1);
  EXPECT_EQ(original.cols, 1);
}

TEST_F(LoadImageTest, LargeImage) {
  cv::Mat largeImg(2000, 3000, CV_8UC3, cv::Scalar(128, 128, 128));
  auto imagePath = tempDir / "large.jpg";
  cv::imwrite(imagePath.string(), largeImg);

  cv::Mat original, undistorted;
  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should handle large image";
  EXPECT_EQ(original.rows, 2000);
  EXPECT_EQ(original.cols, 3000);
}

TEST_F(LoadImageTest, GrayscaleImageLoadsAs3Channel) {
  cv::Mat grayImg(100, 100, CV_8UC1, cv::Scalar(128));
  auto imagePath = tempDir / "gray.png";
  cv::imwrite(imagePath.string(), grayImg);

  cv::Mat original, undistorted;
  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  // OpenCV imread loads as BGR by default
  EXPECT_TRUE(result);
  EXPECT_EQ(original.channels(), 3)
      << "Grayscale should be loaded as 3-channel BGR";
}

// ============== Path with Special Characters ==============

TEST_F(LoadImageTest, PathWithSpaces) {
  auto dirWithSpaces = tempDir / "path with spaces";
  std::filesystem::create_directories(dirWithSpaces);

  cv::Mat img(50, 50, CV_8UC3, cv::Scalar(128, 128, 128));
  auto imagePath = dirWithSpaces / "image.png";
  cv::imwrite(imagePath.string(), img);

  cv::Mat original, undistorted;
  bool result = detect::detail::loadImage(imagePath, original, undistorted);

  EXPECT_TRUE(result) << "Should handle paths with spaces";
}
