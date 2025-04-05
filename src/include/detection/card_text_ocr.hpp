#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace detect {

// Extract text from a card region using OCR
[[nodiscard]] std::string extractText(const cv::Mat& image, std::string language);

// Preprocess image for better OCR results
[[nodiscard]] cv::Mat preprocessForOcr(const cv::Mat& image);

} // namespace detect