#pragma once
#include <opencv2/opencv.hpp>

namespace detect {
// Extract regions from a card image returning the bounding box of each region
[[nodiscard]] cv::Rect extractNameRegion(const cv::Mat &image);
[[nodiscard]] cv::Rect extractCollectorNumberRegionModern(const cv::Mat &image);
[[nodiscard]] cv::Rect extractSetNameRegionModern(const cv::Mat &image);
[[nodiscard]] cv::Rect extractArtRegionRegular(const cv::Mat &image);
[[nodiscard]] cv::Rect extractTextRegion(const cv::Mat &image);

} // namespace detect
