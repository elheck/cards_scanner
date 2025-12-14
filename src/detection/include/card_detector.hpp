#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <vector>

namespace detect {

// Process a card from an image file - this is the only public interface
[[nodiscard]] cv::Mat processCards(const std::filesystem::path& imagePath);

namespace detail {
    // Internal helper functions
    [[nodiscard]] bool loadImage(const std::filesystem::path& imagePath, cv::Mat& originalImage, cv::Mat& undistortedImage);
    void undistortImage(cv::Mat& undistortedImage);
    [[nodiscard]] bool detectCards(const cv::Mat& undistortedImage, std::vector<cv::Mat>& processed_cards);
    [[nodiscard]] cv::Mat warpCard(const std::vector<cv::Point2f>& corners, const cv::Mat& undistortedImage);
    [[nodiscard]] std::vector<cv::Point2f> sortCorners(const std::vector<cv::Point2f>& corners);

    // Normalized card dimensions
    constexpr int normalizedWidth = 480;
    constexpr int normalizedHeight = 680;
}

} // namespace detect