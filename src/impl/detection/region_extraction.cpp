#include <detection/region_extraction.hpp>
#include <spdlog/spdlog.h>

namespace detect {

namespace {
    // Region extraction ratios (relative to card dimensions)
    namespace regions {
        // Name region
        constexpr double name_left_margin = 0.04;   // 4% from left edge
        constexpr double name_top_margin = 0.033;   // 3.3% from top
        constexpr double name_width_ratio = 0.75;   // 75% of card width
        constexpr double name_height_ratio = 0.065; // 6.5% of card height

        // Collector number region
        constexpr double collector_left_ratio = 0.104;  // 10.4% from left
        constexpr double collector_top_ratio = 0.176;   // 17.6% from top
        constexpr double collector_width_ratio = 0.417; // 41.7% of card width
        constexpr double collector_height_ratio = 0.074; // 7.4% of card height

        // Set name region
        constexpr double set_left_ratio = 0.104;    // 10.4% from left
        constexpr double set_top_ratio = 0.279;     // 27.9% from top
        constexpr double set_width_ratio = 0.417;   // 41.7% of card width
        constexpr double set_height_ratio = 0.074;  // 7.4% of card height

        // Art region
        constexpr double art_left_ratio = 0.104;    // 10.4% from left
        constexpr double art_top_ratio = 0.382;     // 38.2% from top
        constexpr double art_width_ratio = 0.792;   // 79.2% of card width
        constexpr double art_height_ratio = 0.221;  // 22.1% of card height

        // Text box region
        constexpr double text_left_ratio = 0.104;   // 10.4% from left
        constexpr double text_top_ratio = 0.618;    // 61.8% from top
        constexpr double text_width_ratio = 0.792;  // 79.2% of card width
        constexpr double text_height_ratio = 0.147; // 14.7% of card height
    }
}

cv::Mat extractNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::name_left_margin);
    int y = static_cast<int>(image.rows * regions::name_top_margin);
    int width = static_cast<int>(image.cols * regions::name_width_ratio);
    int height = static_cast<int>(image.rows * regions::name_height_ratio);

    cv::Rect name_region(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, name_region, cv::Scalar(0, 255, 0), 2);
    return result;
}

cv::Mat extractCollectorNumberRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::collector_left_ratio);
    int y = static_cast<int>(image.rows * regions::collector_top_ratio);
    int width = static_cast<int>(image.cols * regions::collector_width_ratio);
    int height = static_cast<int>(image.rows * regions::collector_height_ratio);

    cv::Rect collector_region(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, collector_region, cv::Scalar(255, 0, 0), 2);
    return result;
}

cv::Mat extractSetNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::set_left_ratio);
    int y = static_cast<int>(image.rows * regions::set_top_ratio);
    int width = static_cast<int>(image.cols * regions::set_width_ratio);
    int height = static_cast<int>(image.rows * regions::set_height_ratio);

    cv::Rect set_region(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, set_region, cv::Scalar(0, 0, 255), 2);
    return result;
}

cv::Mat extractArtRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::art_left_ratio);
    int y = static_cast<int>(image.rows * regions::art_top_ratio);
    int width = static_cast<int>(image.cols * regions::art_width_ratio);
    int height = static_cast<int>(image.rows * regions::art_height_ratio);

    cv::Rect art_region(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, art_region, cv::Scalar(255, 255, 0), 2);
    return result;
}

cv::Mat extractTextRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::text_left_ratio);
    int y = static_cast<int>(image.rows * regions::text_top_ratio);
    int width = static_cast<int>(image.cols * regions::text_width_ratio);
    int height = static_cast<int>(image.rows * regions::text_height_ratio);

    cv::Rect text_region(x, y, width, height);
    cv::Mat result = image.clone();
    cv::rectangle(result, text_region, cv::Scalar(0, 255, 255), 2);
    return result;
}

} // namespace detect