#include <detection/card_processor.hpp>
#include <iostream>
#include <stdexcept>

bool CardProcessor::loadImage(const std::filesystem::path &imagePath) {
  originalImage_ = cv::imread(imagePath.string());
  if (originalImage_.empty()) {
    std::cerr << "Failed to load image: " << imagePath << std::endl;
    return false;
  }

  // Make a copy for processing
  undistortedImage_ = originalImage_.clone();
  return true;
}

cv::Mat CardProcessor::processCards() {
  if (undistortedImage_.empty()) {
    throw std::runtime_error("no cards");
  }

  // Step 1: Undistort the image if you have calibration (no-op here)
  undistortImage();

  // Step 2: Detect all cards
  if (!detectCards()) {
    throw std::runtime_error("no cards detected");
  }

  if (processedCards_.empty()) {
    throw std::runtime_error("Not one card found");
  }

  // If we found at least one card, we're good
  return processedCards_.at(0);
}

void CardProcessor::undistortImage() {
  // In a real application, apply camera calibration here.
  // For now, we’ll just keep the original image.
  // undistortedImage_ = originalImage_.clone();
}

bool CardProcessor::detectCards() {
  processedCards_.clear();

  // Convert to grayscale
  cv::Mat gray;
  cv::cvtColor(undistortedImage_, gray, cv::COLOR_BGR2GRAY);

  // Blur to reduce noise
  cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

  // Adaptive threshold to get a binary image (white = background, black = card
  // edges)
  cv::Mat binary;
  cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

  // Find external contours (to ignore inner details)
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(binary, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  double maxArea = 0.0;
  std::vector<cv::Point2f> bestCorners;

  // Look through all contours to find the largest valid card contour
  for (const auto &cnt : contours) {
    double area = cv::contourArea(cnt);
    if (area < 1000) {
      // Too small to be a card
      continue;
    }

    double perimeter = cv::arcLength(cnt, true);
    std::vector<cv::Point> approx;
    cv::approxPolyDP(cnt, approx, 0.02 * perimeter, true);

    // Check if the approximated contour is a convex quadrilateral
    if (approx.size() == 4 && cv::isContourConvex(approx)) {
      std::vector<cv::Point2f> corners;
      for (const auto &pt : approx) {
        corners.push_back(cv::Point2f(pt.x, pt.y));
      }

      // Keep the candidate with the largest contour area
      if (area > maxArea) {
        maxArea = area;
        bestCorners = corners;
      }
    }
  }

  // If a valid card candidate was found, warp it and store it
  if (!bestCorners.empty()) {
    cv::Mat warped = warpCard(bestCorners);
    if (!warped.empty()) {
      processedCards_.push_back(warped);
    }
  }

  return !processedCards_.empty();
}

std::vector<cv::Point2f>
CardProcessor::sortCorners(const std::vector<cv::Point2f> &corners) {
  // We assume exactly 4 corners.
  // 1) Sort by y, then x
  std::vector<cv::Point2f> sorted = corners;
  std::sort(sorted.begin(), sorted.end(),
            [](const cv::Point2f &a, const cv::Point2f &b) {
              return (a.y < b.y) || (a.y == b.y && a.x < b.x);
            });

  // After this sort:
  //   sorted[0], sorted[1] are the top two (by y)
  //   sorted[2], sorted[3] are the bottom two
  // We still need to check which is left vs. right.

  // The top-left should be sorted[0] if it has smaller x than sorted[1]
  cv::Point2f topLeft, topRight, bottomLeft, bottomRight;
  if (sorted[0].x < sorted[1].x) {
    topLeft = sorted[0];
    topRight = sorted[1];
  } else {
    topLeft = sorted[1];
    topRight = sorted[0];
  }

  // The bottom-left should be sorted[2] if it has smaller x than sorted[3]
  if (sorted[2].x < sorted[3].x) {
    bottomLeft = sorted[2];
    bottomRight = sorted[3];
  } else {
    bottomLeft = sorted[3];
    bottomRight = sorted[2];
  }

  // Return in order: TL, TR, BR, BL
  std::vector<cv::Point2f> dst{topLeft, topRight, bottomRight, bottomLeft};
  return dst;
}

cv::Mat CardProcessor::warpCard(const std::vector<cv::Point2f> &corners) {
  // Ensure corners are in the correct order
  if (corners.size() != 4) {
    return cv::Mat();
  }

  auto sorted = sortCorners(corners);

  // Destination corners for the normalized card
  std::vector<cv::Point2f> dstPoints{
      cv::Point2f(0.f, 0.f), cv::Point2f((float)normalizedWidth_, 0.f),
      cv::Point2f((float)normalizedWidth_, (float)normalizedHeight_),
      cv::Point2f(0.f, (float)normalizedHeight_)};

  // Compute perspective transform
  cv::Mat M = cv::getPerspectiveTransform(sorted, dstPoints);

  // Warp the card
  cv::Mat warped;
  cv::warpPerspective(undistortedImage_, warped, M,
                      cv::Size(normalizedWidth_, normalizedHeight_));
  return warped;
}