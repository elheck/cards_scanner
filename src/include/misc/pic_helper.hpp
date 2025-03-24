#pragma once

#include <opencv2/opencv.hpp>

#include <filesystem> 

namespace cs {
// Show the detected & warped cards (for debugging)
void displayResults(cv::Mat pic);

// Save processed card images to files (instead of showing them)
bool saveImage(const std::filesystem::path &savePath, cv::Mat pic, std::string name = "");
} // namespace cs