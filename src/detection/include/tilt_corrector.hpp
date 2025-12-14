#pragma once

#include <opencv2/opencv.hpp>

namespace detect {

[[nodiscard]] cv::Mat correctCardTilt(const cv::Mat &cardImage);

} // namespace detect