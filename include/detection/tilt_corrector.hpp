#pragma once

#include <opencv2/opencv.hpp>

namespace detect {

cv::Mat correctCardTilt(const cv::Mat &cardImage);

} // namespace detect