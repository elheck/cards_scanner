#include <detection/region_extraction.hpp>
#include <spdlog/spdlog.h>

namespace detect {

namespace {
    // Region extraction ratios (relative to card dimensions)
    namespace regions {
        // Name region
        constexpr double NAME_LEFT_MARGIN = 0.04;   // 4% from left edge
        constexpr double NAME_TOP_MARGIN = 0.033;   // 3.3% from top
        constexpr double NAME_WIDTH_RATIO = 0.75;   // 75% of card width
        constexpr double NAME_HEIGHT_RATIO = 0.065; // 6.5% of card height

        // Collector number region
        constexpr double COLLECTOR_LEFT_RATIO = 0.104;  // 10.4% from left
        constexpr double COLLECTOR_TOP_RATIO = 0.176;   // 17.6% from top
        constexpr double COLLECTOR_WIDTH_RATIO = 0.417; // 41.7% of card width
        constexpr double COLLECTOR_HEIGHT_RATIO = 0.074; // 7.4% of card height

        // Set name region
        constexpr double SET_LEFT_RATIO = 0.104;    // 10.4% from left
        constexpr double SET_TOP_RATIO = 0.279;     // 27.9% from top
        constexpr double SET_WIDTH_RATIO = 0.417;   // 41.7% of card width
        constexpr double SET_HEIGHT_RATIO = 0.074;  // 7.4% of card height

        // Art region
        constexpr double ART_LEFT_RATIO = 0.104;    // 10.4% from left
        constexpr double ART_TOP_RATIO = 0.382;     // 38.2% from top
        constexpr double ART_WIDTH_RATIO = 0.792;   // 79.2% of card width
        constexpr double ART_HEIGHT_RATIO = 0.221;  // 22.1% of card height

        // Text box region
        constexpr double TEXT_LEFT_RATIO = 0.104;   // 10.4% from left
        constexpr double TEXT_TOP_RATIO = 0.618;    // 61.8% from top
        constexpr double TEXT_WIDTH_RATIO = 0.792;  // 79.2% of card width
        constexpr double TEXT_HEIGHT_RATIO = 0.147; // 14.7% of card height
    }
}

cv::Mat extractNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::NAME_LEFT_MARGIN);
    int y = static_cast<int>(image.rows * regions::NAME_TOP_MARGIN);
    int width = static_cast<int>(image.cols * regions::NAME_WIDTH_RATIO);
    int height = static_cast<int>(image.rows * regions::NAME_HEIGHT_RATIO);

    cv::Rect nameRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, nameRegion, cv::Scalar(0, 255, 0), 2);
    return result;
}

cv::Mat extractCollectorNumberRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::COLLECTOR_LEFT_RATIO);
    int y = static_cast<int>(image.rows * regions::COLLECTOR_TOP_RATIO);
    int width = static_cast<int>(image.cols * regions::COLLECTOR_WIDTH_RATIO);
    int height = static_cast<int>(image.rows * regions::COLLECTOR_HEIGHT_RATIO);

    cv::Rect collectorNumberRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, collectorNumberRegion, cv::Scalar(255, 0, 0), 2);
    return result;
}

cv::Mat extractSetNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::SET_LEFT_RATIO);
    int y = static_cast<int>(image.rows * regions::SET_TOP_RATIO);
    int width = static_cast<int>(image.cols * regions::SET_WIDTH_RATIO);
    int height = static_cast<int>(image.rows * regions::SET_HEIGHT_RATIO);

    cv::Rect setNameRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, setNameRegion, cv::Scalar(0, 0, 255), 2);
    return result;
}

cv::Mat extractArtRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::ART_LEFT_RATIO);
    int y = static_cast<int>(image.rows * regions::ART_TOP_RATIO);
    int width = static_cast<int>(image.cols * regions::ART_WIDTH_RATIO);
    int height = static_cast<int>(image.rows * regions::ART_HEIGHT_RATIO);

    cv::Rect artRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, artRegion, cv::Scalar(255, 255, 0), 2);
    return result;
}

cv::Mat extractTextRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::TEXT_LEFT_RATIO);
    int y = static_cast<int>(image.rows * regions::TEXT_TOP_RATIO);
    int width = static_cast<int>(image.cols * regions::TEXT_WIDTH_RATIO);
    int height = static_cast<int>(image.rows * regions::TEXT_HEIGHT_RATIO);

    cv::Rect textRegion(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, textRegion, cv::Scalar(0, 255, 255), 2);
    return result;
}

} // namespace detect