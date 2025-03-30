#include <detection/card_detector.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <cmath>
#include <array>

namespace detect {

namespace {
    // Card detection thresholds
    constexpr double min_card_area_ratio = 0.1;  // Card must be at least 10% of image
    constexpr double card_aspect_ratio = 0.714;  // Standard MTG card ratio (2.5/3.5)
    constexpr double aspect_ratio_tolerance = 0.2;
    constexpr double contour_approx_epsilon = 0.02;

    // Image processing constants
    constexpr int BLUR_RATIO = 100;  // Divisor for calculating blur kernel size
    constexpr int DILATE_RATIO = 67; // Divisor for calculating dilation kernel size
    constexpr double DILATE_ROUND = 0.5; // Rounding constant for dilation
    constexpr int THRESH_RATIO = 20;  // Divisor for calculating threshold kernel size
    constexpr int GAUSSIAN_KERNEL_SIZE = 5;  // Size for Gaussian blur kernel
    constexpr int MAX_PIXEL_VALUE = 255;  // Maximum pixel value for thresholding
    constexpr int THRESH_C_VALUE = 10;  // C value for adaptive threshold
}

namespace detail {

bool loadImage(const std::filesystem::path &imagePath, cv::Mat& originalImage, cv::Mat& undistortedImage) {
    originalImage = cv::imread(imagePath.string());
    if (originalImage.empty()) {
        spdlog::error("Failed to load image: {}", imagePath.string());
        return false;
    }

    // Make a copy for processing
    undistortedImage = originalImage.clone();
    return true;
}

void undistortImage(cv::Mat& undistortedImage) {
    // In a real application, apply camera calibration here.
    // For now, we'll just keep the original image.
}

std::vector<cv::Point2f> sortCorners(const std::vector<cv::Point2f> &corners) {
    // We assume exactly 4 corners.
    // 1) Sort by y, then x
    std::vector<cv::Point2f> sorted = corners;
    std::sort(sorted.begin(), sorted.end(),
            [](const cv::Point2f &a, const cv::Point2f &b) {
                return (a.y < b.y) || (a.y == b.y && a.x < b.x);
            });

    // After this sort:
    //   sorted[0], sorted[1] are the top two (by y)
    //   sorted[2], sorted[3] are the bottom two
    // We still need to check which is left vs. right.

    cv::Point2f top_left;
    cv::Point2f top_right;
    cv::Point2f bottom_left;
    cv::Point2f bottom_right;

    // The top-left should be sorted[0] if it has smaller x than sorted[1]
    if (sorted[0].x < sorted[1].x) {
        top_left = sorted[0];
        top_right = sorted[1];
    } else {
        top_left = sorted[1];
        top_right = sorted[0];
    }

    // The bottom-left should be sorted[2] if it has smaller x than sorted[3]
    if (sorted[2].x < sorted[3].x) {
        bottom_left = sorted[2];
        bottom_right = sorted[3];
    } else {
        bottom_left = sorted[3];
        bottom_right = sorted[2];
    }

    // Return in order: TL, TR, BR, BL
    return {top_left, top_right, bottom_right, bottom_left};
}

cv::Mat warpCard(const std::vector<cv::Point2f> &corners, const cv::Mat& undistortedImage) {
    // Ensure corners are in the correct order
    if (corners.size() != 4) {
        return {};
    }

    auto sorted = sortCorners(corners);

    // Destination corners for the normalized card
    std::vector<cv::Point2f> dst_points{
        cv::Point2f(0.0F, 0.0F), 
        cv::Point2f(static_cast<float>(normalizedWidth), 0.0F),
        cv::Point2f(static_cast<float>(normalizedWidth), static_cast<float>(normalizedHeight)),
        cv::Point2f(0.0F, static_cast<float>(normalizedHeight))};

    // Compute perspective transform
    cv::Mat transform = cv::getPerspectiveTransform(sorted, dst_points);

    // Warp the card
    cv::Mat warped;
    cv::warpPerspective(undistortedImage, warped, transform,
                        cv::Size(normalizedWidth, normalizedHeight));
    return warped;
}

bool detectCards(const cv::Mat& undistortedImage, std::vector<cv::Mat>& processed_cards) {
    processed_cards.clear();

    // Calculate dynamic parameters based on image size
    int min_dim = std::min(undistortedImage.cols, undistortedImage.rows);
    int blur_radius = ((min_dim / BLUR_RATIO + 1) / 2) * 2 + 1;  // Round to nearest odd
    int dilate_radius = static_cast<int>(std::floor(min_dim / DILATE_RATIO + DILATE_ROUND));
    int thresh_radius = ((min_dim / THRESH_RATIO + 1) / 2) * 2 + 1; // Round to nearest odd

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(undistortedImage, gray, cv::COLOR_BGR2GRAY);

    // Apply median blur to better remove background textures
    cv::Mat blurred;
    cv::medianBlur(gray, blurred, blur_radius);

    // Apply Gaussian blur after median blur for better edge detection
    cv::GaussianBlur(blurred, blurred, cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE), 0);

    // Apply adaptive threshold with adjusted parameters
    cv::Mat binary;
    cv::adaptiveThreshold(blurred, binary, MAX_PIXEL_VALUE, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY_INV, thresh_radius, THRESH_C_VALUE);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, 
                                             cv::Size(dilate_radius, dilate_radius));
    
    // First dilate to connect edges
    cv::Mat morphed;
    cv::dilate(binary, morphed, kernel);
    
    // Then erode to clean up noise
    cv::erode(morphed, morphed, kernel);

    // Find contours with modified parameters
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(morphed, contours, hierarchy, cv::RETR_EXTERNAL, 
                    cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return false;
    }

    // Filter contours by area and aspect ratio
    std::vector<std::vector<cv::Point>> valid_contours;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        cv::Rect bound_rect = cv::boundingRect(contour);
        double aspect_ratio = static_cast<double>(bound_rect.width) / bound_rect.height;
        
        if (area > min_card_area_ratio * min_dim * min_dim && 
            std::abs(aspect_ratio - card_aspect_ratio) < aspect_ratio_tolerance) {
            valid_contours.push_back(contour);
        }
    }

    if (valid_contours.empty()) {
        return false;
    }

    // Find the contour with maximum area among valid contours
    auto max_contour = std::max_element(valid_contours.begin(), valid_contours.end(),
        [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
            return cv::contourArea(c1) < cv::contourArea(c2);
        });

    // Approximate the contour to get corners
    std::vector<cv::Point> approx_curve;
    double epsilon = contour_approx_epsilon * cv::arcLength(*max_contour, true);
    cv::approxPolyDP(*max_contour, approx_curve, epsilon, true);

    // Check if we have a valid quadrilateral
    if (approx_curve.size() == 4 && cv::isContourConvex(approx_curve)) {
        std::vector<cv::Point2f> corners;
        corners.reserve(approx_curve.size());  // Pre-allocate capacity
        for (const auto& point : approx_curve) {
            corners.emplace_back(static_cast<float>(point.x), 
                               static_cast<float>(point.y));
        }

        // Sort corners and warp the card
        cv::Mat warped = warpCard(corners, undistortedImage);
        if (!warped.empty()) {
            processed_cards.push_back(warped);
            return true;
        }
    }

    // Alternative: If approximation didn't work, try using the bounding rect
    cv::RotatedRect bounding_box = cv::minAreaRect(*max_contour);
    std::array<cv::Point2f, 4> vertices;
    bounding_box.points(vertices.data());
    
    std::vector<cv::Point2f> corners;
    corners.reserve(4);  // Pre-allocate capacity
    for (const auto& vertex : vertices) {
        corners.push_back(vertex);
    }

    cv::Mat warped = warpCard(corners, undistortedImage);
    if (!warped.empty()) {
        processed_cards.push_back(warped);
        return true;
    }

    return false;
}

} // namespace detail

cv::Mat processCards(const std::filesystem::path& imagePath) {
    cv::Mat original_image;
    cv::Mat undistorted_image;
    std::vector<cv::Mat> processed_cards;

    if (!detail::loadImage(imagePath, original_image, undistorted_image)) {
        throw std::runtime_error("Failed to load image");
    }

    if (undistorted_image.empty()) {
        throw std::runtime_error("no cards");
    }

    // Step 1: Undistort the image if you have calibration (no-op here)
    detail::undistortImage(undistorted_image);

    // Step 2: Detect all cards
    if (!detail::detectCards(undistorted_image, processed_cards)) {
        throw std::runtime_error("no cards detected");
    }

    if (processed_cards.empty()) {
        throw std::runtime_error("Not one card found");
    }

    // If we found at least one card, we're good
    return processed_cards.at(0);
}

} // namespace detect