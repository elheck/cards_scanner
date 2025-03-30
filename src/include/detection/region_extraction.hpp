#pragma once
#include <opencv2/opencv.hpp>

namespace detect
{
    // Extract regions from a card image returning the image with drawn regions
    [[nodiscard]] cv::Mat extractNameRegion(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractCollectorNumberRegionModern(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractSetNameRegionModern(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractArtRegionRegular(const cv::Mat& image);
    [[nodiscard]] cv::Mat extractTextRegion(const cv::Mat& image);

} // namespace detect
