#ifndef CARD_PROCESSOR_HPP
#define CARD_PROCESSOR_HPP

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

class CardProcessor {
public:
  CardProcessor() = default;
  ~CardProcessor() = default;

  // Load an image from file
  bool loadImage(const std::filesystem::path &imagePath);

  // Process the loaded image to normalize the card
  bool processCard();

  // Display the original and processed images
  void displayImages() const;

  // Get the processed card image
  cv::Mat getProcessedCard() const;

private:
  // Undistort the image using camera calibration (if available)
  void undistortImage();

  // Detect the card in the image and extract it
  bool detectCard();

  // Normalize the card to a standard size and orientation
  void normalizeCard();

  // Original image
  cv::Mat originalImage_;

  // Undistorted image
  cv::Mat undistortedImage_;

  // Processed card image
  cv::Mat processedCard_;

  // Card contour points
  std::vector<cv::Point2f> cardCorners_;

  // Standard MTG card aspect ratio (63mm x 88mm)
  const float cardAspectRatio_ = 88.0f / 63.0f;

  // Standard output size for normalized cards
  const int normalizedWidth_ = 630;
  const int normalizedHeight_ = 880;
};

#endif // CARD_PROCESSOR_HPP