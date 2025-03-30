#include <detection/region_extraction.hpp>
#include <spdlog/spdlog.h>

namespace detect {

cv::Mat extractNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * 0.04);   // Start at 4% from left edge
    int y = static_cast<int>(image.rows * 0.033);  // Start at 3.3% from top
    int width = static_cast<int>(image.cols * 0.75);  // Cover 75% of card width
    int height = static_cast<int>(image.rows * 0.065); // Height 6.5% of card height

    // Create the region and draw it on a copy of the original image
    cv::Rect nameRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, nameRegion, cv::Scalar(0, 255, 0), 2); // Draw green rectangle
    return result;
}

cv::Mat extractCollectorNumberRegion(const cv::Mat& image) {
    cv::Rect collectorNumberRegion(50, 120, 200, 50); // Example coordinates
    cv::Mat result = image.clone();
    cv::rectangle(result, collectorNumberRegion, cv::Scalar(255, 0, 0), 2);
    return result;
}

cv::Mat extractSetNameRegion(const cv::Mat& image) {
    cv::Rect setNameRegion(50, 190, 200, 50); // Example coordinates
    cv::Mat result = image.clone();
    cv::rectangle(result, setNameRegion, cv::Scalar(0, 0, 255), 2);
    return result;
}

cv::Mat extractArtRegion(const cv::Mat& image) {
    cv::Rect artRegion(50, 260, 200, 150); // Example coordinates
    cv::Mat result = image.clone();
    cv::rectangle(result, artRegion, cv::Scalar(255, 255, 0), 2);
    return result;
}

cv::Mat extractTextRegion(const cv::Mat& image) {
    cv::Rect textRegion(50, 420, 200, 100); // Example coordinates
    cv::Mat result = image.clone();
    cv::rectangle(result, textRegion, cv::Scalar(0, 255, 255), 2);
    return result;
}

} // namespace detect