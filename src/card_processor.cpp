#include "card_processor.hpp"
#include <iostream>

bool CardProcessor::loadImage(const std::filesystem::path &imagePath) {
  originalImage_ = cv::imread(imagePath);
  if (originalImage_.empty()) {
    std::cerr << "Failed to load image: " << imagePath << std::endl;
    return false;
  }

  // Make a copy for processing
  undistortedImage_ = originalImage_.clone();
  return true;
}

bool CardProcessor::processCard() {
  if (undistortedImage_.empty()) {
    std::cerr << "No image loaded" << std::endl;
    return false;
  }

  // Step 1: Undistort the image (no-op if no calibration data)
  undistortImage();

  // Step 2: Detect the card
  if (!detectCard()) {
    std::cerr << "Failed to detect card" << std::endl;
    return false;
  }

  // Step 3: Normalize the card
  normalizeCard();

  return !processedCard_.empty();
}

void CardProcessor::undistortImage() {
  // In a real application, you would apply camera calibration here
  // For simplicity, we'll just use the original image
  // undistortedImage_ = originalImage_.clone();
}

bool CardProcessor::detectCard() {
  // Convert to grayscale
  cv::Mat gray;
  cv::cvtColor(undistortedImage_, gray, cv::COLOR_BGR2GRAY);

  // Apply Gaussian blur to reduce noise
  cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

  // Apply adaptive threshold to get a binary image
  cv::Mat binary;
  cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                        cv::THRESH_BINARY_INV, 11, 2);

  // Find contours
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

  // Find the largest contour that could be a card
  double maxArea = 0;
  int maxAreaIdx = -1;

  for (int i = 0; i < contours.size(); i++) {
    double area = cv::contourArea(contours[i]);

    // Filter by area - cards should be relatively large in the image
    if (area > 1000) {
      std::vector<cv::Point> approx;
      cv::approxPolyDP(contours[i], approx,
                       cv::arcLength(contours[i], true) * 0.02, true);

      // Cards are quadrilaterals
      if (approx.size() == 4 && cv::isContourConvex(approx)) {
        if (area > maxArea) {
          maxArea = area;
          maxAreaIdx = i;

          // Convert to Point2f for perspective transform
          cardCorners_.clear();
          for (const auto &point : approx) {
            cardCorners_.push_back(cv::Point2f(point.x, point.y));
          }
        }
      }
    }
  }

  // If we found a card, draw it on the image for visualization
  if (maxAreaIdx >= 0) {
    cv::Mat debugImage = undistortedImage_.clone();
    cv::drawContours(debugImage, contours, maxAreaIdx, cv::Scalar(0, 255, 0),
                     2);

    // Draw the corners
    for (const auto &corner : cardCorners_) {
      cv::circle(debugImage, corner, 5, cv::Scalar(0, 0, 255), -1);
    }
    return true;
  }

  return false;
}

void CardProcessor::normalizeCard() {
  if (cardCorners_.size() != 4) {
    std::cerr << "Invalid card corners" << std::endl;
    return;
  }

  // Sort the corners to ensure they are in the correct order:
  // 0: top-left, 1: top-right, 2: bottom-right, 3: bottom-left

  // First, find the center of the corners
  cv::Point2f center(0, 0);
  for (const auto &corner : cardCorners_) {
    center += corner;
  }
  center.x /= 4;
  center.y /= 4;

  // Sort corners based on their position relative to the center
  std::vector<cv::Point2f> sortedCorners(4);

  for (const auto &corner : cardCorners_) {
    if (corner.x < center.x && corner.y < center.y) {
      sortedCorners[0] = corner; // top-left
    } else if (corner.x > center.x && corner.y < center.y) {
      sortedCorners[1] = corner; // top-right
    } else if (corner.x > center.x && corner.y > center.y) {
      sortedCorners[2] = corner; // bottom-right
    } else if (corner.x < center.x && corner.y > center.y) {
      sortedCorners[3] = corner; // bottom-left
    }
  }

  // Define the destination points for the perspective transform
  std::vector<cv::Point2f> dstPoints = {
      cv::Point2f(0, 0),                                // top-left
      cv::Point2f(normalizedWidth_, 0),                 // top-right
      cv::Point2f(normalizedWidth_, normalizedHeight_), // bottom-right
      cv::Point2f(0, normalizedHeight_)                 // bottom-left
  };

  // Calculate the perspective transform matrix
  cv::Mat perspectiveMatrix =
      cv::getPerspectiveTransform(sortedCorners, dstPoints);

  // Apply the perspective transformation
  cv::warpPerspective(undistortedImage_, processedCard_, perspectiveMatrix,
                      cv::Size(normalizedWidth_, normalizedHeight_));
}

void CardProcessor::displayImages() const {
  if (!processedCard_.empty()) {
    cv::imshow("Processed Card", processedCard_);
  }

  cv::waitKey(0);
}

cv::Mat CardProcessor::getProcessedCard() const { return processedCard_; }