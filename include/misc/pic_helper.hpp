#pragma once

#include <opencv2/opencv.hpp>

namespace cs {
// Show the detected & warped cards (for debugging)
void displayResults(cv::Mat pic);

// Save processed card images to files (instead of showing them)
bool saveResults(const std::filesystem::path &originalPath, cv::Mat pic);
} // namespace cs