#include <detection/card_detector.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <cmath>
#include <array>

namespace detect {

namespace {
    // Card detection thresholds
    constexpr double MIN_CARD_AREA_RATIO = 0.1;  // Card must be at least 10% of image
    constexpr double CARD_ASPECT_RATIO = 0.714;  // Standard MTG card ratio (2.5/3.5)
    constexpr double ASPECT_RATIO_TOLERANCE = 0.2;
    constexpr double CONTOUR_APPROX_EPSILON = 0.02;
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

    // The top-left should be sorted[0] if it has smaller x than sorted[1]
    cv::Point2f topLeft, topRight, bottomLeft, bottomRight;
    if (sorted[0].x < sorted[1].x) {
        topLeft = sorted[0];
        topRight = sorted[1];
    } else {
        topLeft = sorted[1];
        topRight = sorted[0];
    }

    // The bottom-left should be sorted[2] if it has smaller x than sorted[3]
    if (sorted[2].x < sorted[3].x) {
        bottomLeft = sorted[2];
        bottomRight = sorted[3];
    } else {
        bottomLeft = sorted[3];
        bottomRight = sorted[2];
    }

    // Return in order: TL, TR, BR, BL
    return {topLeft, topRight, bottomRight, bottomLeft};
}

cv::Mat warpCard(const std::vector<cv::Point2f> &corners, const cv::Mat& undistortedImage) {
    // Ensure corners are in the correct order
    if (corners.size() != 4) {
        return cv::Mat();
    }

    auto sorted = sortCorners(corners);

    // Destination corners for the normalized card
    std::vector<cv::Point2f> dstPoints{
        cv::Point2f(0.f, 0.f), 
        cv::Point2f((float)normalizedWidth, 0.f),
        cv::Point2f((float)normalizedWidth, (float)normalizedHeight),
        cv::Point2f(0.f, (float)normalizedHeight)};

    // Compute perspective transform
    cv::Mat transform = cv::getPerspectiveTransform(sorted, dstPoints);

    // Warp the card
    cv::Mat warped;
    cv::warpPerspective(undistortedImage, warped, transform,
                        cv::Size(normalizedWidth, normalizedHeight));
    return warped;
}

bool detectCards(const cv::Mat& undistortedImage, std::vector<cv::Mat>& processed_cards) {
    processed_cards.clear();

    // Calculate dynamic parameters based on image size
    int minDim = std::min(undistortedImage.cols, undistortedImage.rows);
    int blurRadius = ((minDim / 100 + 1) / 2) * 2 + 1;  // Round to nearest odd
    int dilateRadius = static_cast<int>(std::floor(minDim / 67.0 + 0.5));
    int threshRadius = ((minDim / 20 + 1) / 2) * 2 + 1; // Round to nearest odd

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(undistortedImage, gray, cv::COLOR_BGR2GRAY);

    // Apply median blur to better remove background textures
    cv::Mat blurred;
    cv::medianBlur(gray, blurred, blurRadius);

    // Apply Gaussian blur after median blur for better edge detection
    cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), 0);

    // Apply adaptive threshold with adjusted parameters
    cv::Mat binary;
    cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY_INV, threshRadius, 10);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, 
                                             cv::Size(dilateRadius, dilateRadius));
    
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
    std::vector<std::vector<cv::Point>> validContours;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        cv::Rect boundRect = cv::boundingRect(contour);
        double aspectRatio = static_cast<double>(boundRect.width) / boundRect.height;
        
        if (area > MIN_CARD_AREA_RATIO * minDim * minDim && 
            std::abs(aspectRatio - CARD_ASPECT_RATIO) < ASPECT_RATIO_TOLERANCE) {
            validContours.push_back(contour);
        }
    }

    if (validContours.empty()) {
        return false;
    }

    // Find the contour with maximum area among valid contours
    auto maxContour = std::max_element(validContours.begin(), validContours.end(),
        [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
            return cv::contourArea(c1) < cv::contourArea(c2);
        });

    // Approximate the contour to get corners
    std::vector<cv::Point> approxCurve;
    double epsilon = CONTOUR_APPROX_EPSILON * cv::arcLength(*maxContour, true);
    cv::approxPolyDP(*maxContour, approxCurve, epsilon, true);

    // Check if we have a valid quadrilateral
    if (approxCurve.size() == 4 && cv::isContourConvex(approxCurve)) {
        std::vector<cv::Point2f> corners;
        for (const auto& point : approxCurve) {
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
    cv::RotatedRect bounding_box = cv::minAreaRect(*maxContour);
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