// ImageProcessor.cpp
#include <algorithm>
#include <core/imageProcessor.hpp>
#include <stdexcept>
#include <vector>

ImageProcessor::ImageProcessor(const cv::Mat &inputImage,
                               const cv::Mat &cameraMatrix,
                               const cv::Mat &distCoeffs)
    : originalImage_(inputImage.clone()), cameraMatrix_(cameraMatrix),
      distCoeffs_(distCoeffs) {}

void ImageProcessor::processCard() {
  if (originalImage_.empty())
    return;

  // Image correction pipeline
  cv::undistort(originalImage_, undistortedImage_, cameraMatrix_, distCoeffs_);
  extractCardArea();
  normalizeCardSize();

  // Information extraction
  extractArtwork();
  extractTextRegions();
}

// Getters implementation
cv::Mat ImageProcessor::getProcessedCard() const { return normalizedCard_; }

void ImageProcessor::extractCardArea() {
  cv::Mat gray, blurred, thresh;
  cv::cvtColor(undistortedImage_, gray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
  cv::threshold(blurred, thresh, 0, 255,
                cv::THRESH_BINARY_INV + cv::THRESH_OTSU);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  std::sort(contours.begin(), contours.end(), [](const auto &a, const auto &b) {
    return cv::contourArea(a) > cv::contourArea(b);
  });

  if (contours.size() < 2)
    return;

  auto cardContour = contours[1];
  auto rect = cv::minAreaRect(cardContour);
  cv::Point2f srcPoints[4];
  rect.points(srcPoints);

  cv::Point2f dstPoints[4] = {{0, 0},
                              {rect.size.width, 0},
                              {rect.size.width, rect.size.height},
                              {0, rect.size.height}};

  cv::Mat transform = cv::getPerspectiveTransform(srcPoints, dstPoints);
  cv::warpPerspective(undistortedImage_, normalizedCard_, transform, rect.size);
}

void ImageProcessor::normalizeCardSize() {
  if (normalizedCard_.empty()) {
    throw std::runtime_error(
        "normalizedCard_ is empty. Card area extraction failed.");
  }
  const cv::Size targetSize(630, 880);
  cv::resize(normalizedCard_, normalizedCard_, targetSize);
}

void ImageProcessor::extractArtwork() {
  cv::Rect artworkROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.15,
                      normalizedCard_.cols * 0.85, normalizedCard_.rows * 0.60);
  artworkRegion_ = normalizedCard_(artworkROI);
}

void ImageProcessor::extractTextRegions() {
  // Name region
  cv::Rect nameROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.05,
                   normalizedCard_.cols * 0.45, normalizedCard_.rows * 0.08);
  nameRegion_ = normalizedCard_(nameROI);

  // Set information region
  cv::Rect setROI(normalizedCard_.cols * 0.65, normalizedCard_.rows * 0.80,
                  normalizedCard_.cols * 0.25, normalizedCard_.rows * 0.10);
  setRegion_ = normalizedCard_(setROI);

  // Collector number region
  cv::Rect numberROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.90,
                     normalizedCard_.cols * 0.15, normalizedCard_.rows * 0.06);
  numberRegion_ = normalizedCard_(numberROI);

  // Language region
  cv::Rect languageROI(normalizedCard_.cols * 0.80, normalizedCard_.rows * 0.93,
                       normalizedCard_.cols * 0.15,
                       normalizedCard_.rows * 0.06);
  languageRegion_ = normalizedCard_(languageROI);
}

void ImageProcessor::displayCardParts() const {
  // Create a copy of normalizedCard_ to draw bounding boxes on.
  cv::Mat annotatedCard = normalizedCard_.clone();

  // Re-calculate the ROIs as defined in extractArtwork() and
  // extractTextRegions()
  cv::Rect artworkROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.15,
                      normalizedCard_.cols * 0.85, normalizedCard_.rows * 0.60);
  cv::Rect nameROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.05,
                   normalizedCard_.cols * 0.45, normalizedCard_.rows * 0.08);
  cv::Rect setROI(normalizedCard_.cols * 0.65, normalizedCard_.rows * 0.80,
                  normalizedCard_.cols * 0.25, normalizedCard_.rows * 0.10);
  cv::Rect numberROI(normalizedCard_.cols * 0.05, normalizedCard_.rows * 0.90,
                     normalizedCard_.cols * 0.15, normalizedCard_.rows * 0.06);
  cv::Rect languageROI(normalizedCard_.cols * 0.80, normalizedCard_.rows * 0.93,
                       normalizedCard_.cols * 0.15,
                       normalizedCard_.rows * 0.06);

  // Draw bounding boxes on the full card image
  cv::rectangle(annotatedCard, artworkROI, cv::Scalar(0, 255, 0), 2);
  cv::rectangle(annotatedCard, nameROI, cv::Scalar(255, 0, 0), 2);
  cv::rectangle(annotatedCard, setROI, cv::Scalar(0, 0, 255), 2);
  cv::rectangle(annotatedCard, numberROI, cv::Scalar(255, 255, 0), 2);
  cv::rectangle(annotatedCard, languageROI, cv::Scalar(255, 0, 255), 2);

  // Show the annotated full card image
  cv::imshow("Normalized Card with Regions", annotatedCard);

  // Show each extracted region in separate windows
  cv::imshow("Artwork Region", artworkRegion_);
  cv::imshow("Name Region", nameRegion_);
  cv::imshow("Set Region", setRegion_);
  cv::imshow("Number Region", numberRegion_);
  cv::imshow("Language Region", languageRegion_);

  cv::waitKey(0); // Wait until a key is pressed
}
