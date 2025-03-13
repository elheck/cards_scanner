#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <vector>

class CardProcessor {
public:
  // Load the original image
  bool loadImage(const std::filesystem::path &imagePath);

  // Process *all* cards found in the image
  bool processCards();

  // Show the detected & warped cards (for debugging)
  void displayResults() const;

  // Save processed card images to files (instead of showing them)
  bool saveResults(const std::filesystem::path &originalPath);

private:
  // Internal helpers
  void undistortImage(); // no-op unless you have camera calibration
  bool detectCards();    // find all quadrilaterals that could be cards
  cv::Mat warpCard(const std::vector<cv::Point2f> &corners);

  // Helper to reorder corners: top-left, top-right, bottom-right, bottom-left
  std::vector<cv::Point2f> sortCorners(const std::vector<cv::Point2f> &corners);

private:
  // Input images
  cv::Mat originalImage_;
  cv::Mat undistortedImage_;

  // Output for each detected card
  std::vector<cv::Mat> processedCards_;

  // Desired card size after perspective transform
  static constexpr int normalizedWidth_ = 480;
  static constexpr int normalizedHeight_ = 680;
};
