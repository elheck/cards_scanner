#include <detection/card_detector.hpp>
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace detect {

bool CardDetector::loadImage(const std::filesystem::path &imagePath) {
  originalImage_ = cv::imread(imagePath.string());
  if (originalImage_.empty()) {
    std::cerr << "Failed to load image: " << imagePath << std::endl;
    return false;
  }

  // Make a copy for processing
  undistortedImage_ = originalImage_.clone();
  return true;
}

cv::Mat CardDetector::processCards() {
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

void CardDetector::undistortImage() {
  // In a real application, apply camera calibration here.
  // For now, we’ll just keep the original image.
  // undistortedImage_ = originalImage_.clone();
}

bool CardDetector::detectCards() {
    processedCards_.clear();

    // Calculate dynamic parameters based on image size
    int minDim = std::min(undistortedImage_.cols, undistortedImage_.rows);
    int blurRadius = ((minDim / 100 + 1) / 2) * 2 + 1;
    int dilateRadius = static_cast<int>(std::floor(minDim / 67.0 + 0.5));
    int threshRadius = ((minDim / 20 + 1) / 2) * 2 + 1;

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(undistortedImage_, gray, cv::COLOR_BGR2GRAY);

    // Enhance contrast using CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8,8));
    cv::Mat enhanced;
    clahe->apply(gray, enhanced);

    // Apply bilateral filter to preserve edges while reducing noise
    cv::Mat filtered;
    cv::bilateralFilter(enhanced, filtered, 9, 75, 75);

    // Apply adaptive threshold with adjusted parameters
    cv::Mat binary;
    cv::adaptiveThreshold(filtered, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY_INV, threshRadius, 8);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, 
                                             cv::Size(dilateRadius, dilateRadius));
    
    // First dilate to connect edges
    cv::Mat morphed;
    cv::dilate(binary, morphed, kernel);
    
    // Then erode to clean up noise
    cv::erode(morphed, morphed, kernel);

    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(morphed, contours, hierarchy, cv::RETR_EXTERNAL, 
                    cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return false;
    }

    // Filter contours by area and aspect ratio
    std::vector<std::vector<cv::Point>> validContours;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        cv::Rect boundRect = cv::boundingRect(contour);
        double aspectRatio = static_cast<double>(boundRect.width) / boundRect.height;
        
        // Magic card aspect ratio is approximately 2.5/3.5 ≈ 0.714
        const double targetAspectRatio = 0.714;
        const double aspectRatioTolerance = 0.1; // Stricter tolerance

        // Area should be at least 15% of the image and aspect ratio should be close to Magic card ratio
        if (area > 0.15 * minDim * minDim && 
            std::abs(aspectRatio - targetAspectRatio) < aspectRatioTolerance) {
            validContours.push_back(contour);
        }
    }

    if (validContours.empty()) {
        return false;
    }

    // Find the contour with maximum area among valid contours
    auto maxContour = std::max_element(validContours.begin(), validContours.end(),
        [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
            return cv::contourArea(c1) < cv::contourArea(c2);
        });

    // Approximate the contour with higher precision
    std::vector<cv::Point> approxCurve;
    double epsilon = 0.01 * cv::arcLength(*maxContour, true); // More precise approximation
    cv::approxPolyDP(*maxContour, approxCurve, epsilon, true);

    // Check if we have a valid quadrilateral
    if (approxCurve.size() == 4 && cv::isContourConvex(approxCurve)) {
        std::vector<cv::Point2f> corners;
        for (const auto& point : approxCurve) {
            corners.emplace_back(static_cast<float>(point.x), 
                               static_cast<float>(point.y));
        }

        // Sort corners and warp the card
        cv::Mat warped = warpCard(corners);
        if (!warped.empty()) {
            processedCards_.push_back(warped);
            return true;
        }
    }

    // Fallback to rotated rectangle if approximation failed
    cv::RotatedRect boundingBox = cv::minAreaRect(*maxContour);
    cv::Point2f vertices[4];
    boundingBox.points(vertices);
    
    std::vector<cv::Point2f> corners;
    for (int i = 0; i < 4; ++i) {
        corners.push_back(vertices[i]);
    }

    cv::Mat warped = warpCard(corners);
    if (!warped.empty()) {
        processedCards_.push_back(warped);
        return true;
    }

    return false;
}

std::vector<cv::Point2f>
CardDetector::sortCorners(const std::vector<cv::Point2f> &corners) {
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

cv::Mat CardDetector::warpCard(const std::vector<cv::Point2f> &corners) {
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

} // namespace detect