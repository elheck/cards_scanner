#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace detect {

// Extract text from a card region using OCR
[[nodiscard]] std::string extractText(const cv::Mat &image,
                                      const std::string &language = "eng");

// Extract collector number (digits only)
[[nodiscard]] std::string
extractCollectorNumber(const cv::Mat &image,
                       const std::string &language = "eng");

// Extract set code (3-letter uppercase)
[[nodiscard]] std::string extractSetCode(const cv::Mat &image,
                                         const std::string &language = "eng");

// Preprocess image for better OCR results
[[nodiscard]] cv::Mat preprocessForOcr(const cv::Mat &image);

} // namespace detect