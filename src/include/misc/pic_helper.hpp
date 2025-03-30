#pragma once

#include <opencv2/opencv.hpp>

#include <filesystem> 

namespace misc {
// Show the detected & warped cards (for debugging)
void displayResults(cv::Mat pic);

// Save processed card images to files (instead of showing them)
[[nodiscard]] bool saveImage(const std::filesystem::path &savePath, cv::Mat pic, std::string name = "");

// Check if the image is valid (non-empty and 3 channels)
void checkImage(const cv::Mat &pic, const std::string &operationName);
} // namespace misc