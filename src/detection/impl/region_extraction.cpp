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

        // Collector number region (bottom left corner)
        constexpr double collector_left_ratio = 0.04;   // 4% from left
        constexpr double collector_top_ratio = 0.93;    // 93% from top
        constexpr double collector_width_ratio = 0.15;  // 15% of card width
        constexpr double collector_height_ratio = 0.04; // 4% of card height

        // Set name region (modern layout - 3 letter abbreviation)
        constexpr double set_left_ratio = 0.04;    // 4% from left (same as collector number)
        constexpr double set_top_ratio = 0.96;     // 96% from top (just below collector number)
        constexpr double set_width_ratio = 0.10;   // 15% of card width (same as collector number)
        constexpr double set_height_ratio = 0.025; // 2.5% of card height (smaller than collector number)

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

        // Drawing constants
        constexpr int max_color = 255;  // Maximum RGB value
        constexpr int rect_thickness = 2;  // Rectangle border thickness
    }

    // Region extraction parameters
    namespace art_detect {
        constexpr int canny_threshold1 = 30;        // Lower threshold for edge detection
        constexpr int canny_threshold2 = 90;        // Upper threshold
        constexpr int blur_size = 3;               // Bilateral filter size
        constexpr double epsilon_factor = 0.02;     // For approxPolyDP
        constexpr int min_vertices = 4;            // Minimum vertices for art region
        constexpr int max_vertices = 8;            // Maximum vertices for art region
        constexpr double min_art_area_ratio = 0.15; // Min area as fraction of card
        constexpr double max_art_area_ratio = 0.55; // Max area as fraction of card
        constexpr double expected_art_y_position = 0.35; // Expected center Y position
        constexpr double y_position_tolerance = 0.25; // Tolerance for Y position
        constexpr int morph_size = 3;              // Size for morphological operations
        constexpr double min_contour_area = 1000.0; // Minimum contour area in pixels
    }
}

cv::Rect extractNameRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::name_left_margin);
    int y = static_cast<int>(image.rows * regions::name_top_margin);
    int width = static_cast<int>(image.cols * regions::name_width_ratio);
    int height = static_cast<int>(image.rows * regions::name_height_ratio);

    return {x, y, width, height};
}

cv::Rect extractCollectorNumberRegionModern(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::collector_left_ratio);
    int y = static_cast<int>(image.rows * regions::collector_top_ratio);
    int width = static_cast<int>(image.cols * regions::collector_width_ratio);
    int height = static_cast<int>(image.rows * regions::collector_height_ratio);

    return {x, y, width, height};
}

cv::Rect extractSetNameRegionModern(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::set_left_ratio);
    int y = static_cast<int>(image.rows * regions::set_top_ratio);
    int width = static_cast<int>(image.cols * regions::set_width_ratio);
    int height = static_cast<int>(image.rows * regions::set_height_ratio);

    return {x, y, width, height};
}

cv::Rect extractArtRegionRegular(const cv::Mat& image) {
    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Apply Gaussian blur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

    // Edge detection using Canny
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 150);

    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Variables to store the best contour
    std::vector<cv::Point> best_contour;
    double best_score = 0.0;

    // Iterate through contours to find the best candidate for the artwork box
    for (const auto& contour : contours) {
        // Approximate the contour to a polygon
        std::vector<cv::Point> approx;
        double epsilon = 0.02 * cv::arcLength(contour, true);
        cv::approxPolyDP(contour, approx, epsilon, true);

        // Check if the polygon is a rectangle (4 vertices and convex)
        if (approx.size() == 4 && cv::isContourConvex(approx)) {
            // Calculate the bounding box and aspect ratio
            cv::Rect bbox = cv::boundingRect(approx);
            double aspect_ratio = static_cast<double>(bbox.width) / bbox.height;

            // Filter based on aspect ratio (artwork boxes are roughly square or rectangular)
            if (aspect_ratio > 0.8 && aspect_ratio < 1.5) {
                // Calculate the area of the contour and the bounding box
                double contour_area = cv::contourArea(approx);
                double bbox_area = bbox.width * bbox.height;

                // Calculate a score based on how well the contour fits the bounding box
                double score = contour_area / bbox_area;

                // Update the best contour if this one has a higher score
                if (score > best_score) {
                    best_score = score;
                    best_contour = approx;
                }
            }
        }
    }

    // Return the bounding box of the best contour if found
    if (!best_contour.empty()) {
        return cv::boundingRect(best_contour);
    }

    // Return an empty rectangle if no valid contour is found
    return {};
}

cv::Rect extractTextRegion(const cv::Mat& image) {
    int x = static_cast<int>(image.cols * regions::text_left_ratio);
    int y = static_cast<int>(image.rows * regions::text_top_ratio);
    int width = static_cast<int>(image.cols * regions::text_width_ratio);
    int height = static_cast<int>(image.rows * regions::text_height_ratio);

    return {x, y, width, height};
}

} // namespace detect