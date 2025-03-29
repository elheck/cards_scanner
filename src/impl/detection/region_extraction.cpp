#include <detection/region_extraction.hpp>
#include <spdlog/spdlog.h>

namespace detect {

RegionExtractor::RegionExtractor(cv::Mat &image) : image_(image) {}

RegionExtractor::~RegionExtractor() = default;

cv::Mat RegionExtractor::extractNameRegion() {
    // Magic card name dimensions:
    // - Keep at 3% from left edge
    // - Start at 3% from top
    // - Cover 75% of width (reduced from 80%)
    // - Height about 6% of card height
    int x = static_cast<int>(image_.cols * 0.03);   // Start at 3% from left edge
    int y = static_cast<int>(image_.rows * 0.03);   // Start at 3% from top
    int width = static_cast<int>(image_.cols * 0.75); // Cover 75% of card width
    int height = static_cast<int>(image_.rows * 0.06); // Height 6% of card height

    // Extract the region
    cv::Rect nameRegion(x, y, width, height);
    return image_(nameRegion).clone();
}

cv::Mat RegionExtractor::extractCollectorNumberRegion() {
    cv::Rect collectorNumberRegion(50, 120, 200, 50); // Example coordinates
    cv::Mat result = image_.clone();
    cv::rectangle(result, collectorNumberRegion, cv::Scalar(255, 0, 0), 2);
    return result;
}

cv::Mat RegionExtractor::extarctSetNameRegion() {
    cv::Rect setNameRegion(50, 190, 200, 50); // Example coordinates
    cv::Mat result = image_.clone();
    cv::rectangle(result, setNameRegion, cv::Scalar(0, 0, 255), 2);
    return result;
}

cv::Mat RegionExtractor::extractArtRegion() {
    cv::Rect artRegion(50, 260, 200, 150); // Example coordinates
    cv::Mat result = image_.clone();
    cv::rectangle(result, artRegion, cv::Scalar(255, 255, 0), 2);
    return result;
}

cv::Mat RegionExtractor::extractTextRegion() {
    cv::Rect textRegion(50, 420, 200, 100); // Example coordinates
    cv::Mat result = image_.clone();
    cv::rectangle(result, textRegion, cv::Scalar(0, 255, 255), 2);
    return result;
}

} // namespace detect