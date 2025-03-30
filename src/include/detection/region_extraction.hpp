#pragma once
#include <opencv2/opencv.hpp>

namespace detect
{
    // Extract regions from a card image returning the image with drawn regions
    [[nodiscard]] cv::Mat extractNameRegion(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractCollectorNumberRegion(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractSetNameRegion(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractArtRegion(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractTextRegion(const cv::Mat& image);

} // namespace detect
