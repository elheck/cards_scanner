#include <detection/tilt_corrector.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>

namespace detect {

namespace {
    // Image processing parameters
    constexpr int GAUSSIAN_KERNEL_SIZE = 5;
    constexpr double CANNY_THRESHOLD_LOW = 50.0;
    constexpr double CANNY_THRESHOLD_HIGH = 150.0;
    constexpr int GAUSSIAN_SIGMA = 0;  // 0 means auto-compute
}

cv::Mat correctCardTilt(const cv::Mat &cardImage) {
    // Step 1: Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(cardImage, gray, cv::COLOR_BGR2GRAY);

    // Step 2: Apply GaussianBlur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, 
                     cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                     GAUSSIAN_SIGMA);

    // Step 3: Use edge detection (Canny)
    cv::Mat edges;
    cv::Canny(blurred, edges, CANNY_THRESHOLD_LOW, CANNY_THRESHOLD_HIGH);

    // Step 4: Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Step 5: Find the largest contour (assuming it's the card)
    double maxArea = 0.0;
    std::vector<cv::Point> largestContour;
    for (const auto &contour : contours) {
        double area = cv::contourArea(contour);
        if (area > maxArea) {
            maxArea = area;
            largestContour = contour;
        }
    }

    if (largestContour.empty()) {
        // No significant contour detected
        return cardImage.clone();
    }

    // Step 6: Fit a rotated rectangle around the largest contour
    cv::RotatedRect boundingBox = cv::minAreaRect(largestContour);

    // Step 7: Calculate the tilt angle
    double tiltAngle = boundingBox.angle;
    if (boundingBox.size.width < boundingBox.size.height) {
        tiltAngle += 90.0;
    }

    // Step 8: Rotate the image to correct tilt
    cv::Point2f center(cardImage.cols / 2.0, cardImage.rows / 2.0);
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, tiltAngle, 1.0);

    cv::Mat correctedImage;
    cv::warpAffine(cardImage, correctedImage, rotationMatrix, cardImage.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    return correctedImage;
}

} // namespace detect