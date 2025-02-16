// ImageProcessor.hpp
#pragma once

#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

class ImageProcessor {
public:
  ImageProcessor(const cv::Mat &inputImage,
                 const cv::Mat &cameraMatrix = cv::Mat::eye(3, 3, CV_64F),
                 const cv::Mat &distCoeffs = cv::Mat::zeros(5, 1, CV_64F));

  void processCard();

  // Getters
  cv::Mat getProcessedCard() const;

  // New function to visualize the extracted parts
  void displayCardParts() const;

private:
  // Image processing components
  cv::Mat originalImage_;
  cv::Mat undistortedImage_;
  cv::Mat normalizedCard_;
  cv::Mat artworkRegion_;
  cv::Mat nameRegion_;
  cv::Mat setRegion_;
  cv::Mat numberRegion_;
  cv::Mat languageRegion_;

  // Camera parameters
  cv::Mat cameraMatrix_;
  cv::Mat distCoeffs_;

  // Processing methods
  void initOCR();
  void extractCardArea();
  void normalizeCardSize();
  void extractArtwork();
  void extractTextRegions();
  std::string processTextRegion(cv::Mat region);
};
