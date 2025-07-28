#include <tilt_corrector.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <gsl/narrow>
#include <cmath>

namespace detect {

namespace {
    // Image processing parameters
    constexpr int gaussian_kernel_size = 5;
    constexpr double canny_threshold_low = 50.0;
    constexpr double canny_threshold_high = 150.0;
    constexpr int gaussian_sigma = 0;  // 0 means auto-compute
    constexpr double center_divisor = 2.0;  // Used to find image center
    constexpr double ninety_degrees = 90.0; // Rotation adjustment for portrait orientation
}

cv::Mat correctCardTilt(const cv::Mat &cardImage) {
    // Step 1: Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(cardImage, gray, cv::COLOR_BGR2GRAY);

    // Step 2: Apply GaussianBlur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, 
                     cv::Size(gaussian_kernel_size, gaussian_kernel_size),
                     gaussian_sigma);

    // Step 3: Use edge detection (Canny)
    cv::Mat edges;
    cv::Canny(blurred, edges, canny_threshold_low, canny_threshold_high);

    // Step 4: Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Step 5: Find the largest contour (assuming it's the card)
    double max_area = 0.0;
    std::vector<cv::Point> largest_contour;
    for (const auto &contour : contours) {
        double area = cv::contourArea(contour);
        if (area > max_area) {
            max_area = area;
            largest_contour = contour;
        }
    }

    if (largest_contour.empty()) {
        // No significant contour detected
        return cardImage.clone();
    }

    // Step 6: Fit a rotated rectangle around the largest contour
    cv::RotatedRect bounding_box = cv::minAreaRect(largest_contour);

    // Step 7: Calculate the tilt angle
    double tilt_angle = bounding_box.angle;
    if (bounding_box.size.width < bounding_box.size.height) {
        tilt_angle += ninety_degrees;
    }

    // Step 8: Rotate the image to correct tilt
    cv::Point2f center(
        gsl::narrow_cast<float>(cardImage.cols / center_divisor),
        gsl::narrow_cast<float>(cardImage.rows / center_divisor)
    );
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, tilt_angle, 1.0);

    cv::Mat corrected_image;
    cv::warpAffine(cardImage, corrected_image, rotation_matrix, cardImage.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    return corrected_image;
}

} // namespace detect