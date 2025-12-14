#include <card_text_ocr.hpp>
#include <spdlog/spdlog.h>

namespace detect {

cv::Mat preprocessForOcr(const cv::Mat& image) {
    cv::Mat processed;
    
    // Convert to grayscale if the image is in color
    if (image.channels() == 3) {
        cv::cvtColor(image, processed, cv::COLOR_BGR2GRAY);
    } else {
        processed = image.clone();
    }

    // Apply adaptive thresholding to handle varying lighting conditions
    cv::adaptiveThreshold(processed, processed, 255,
                         cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY, 11, 2);

    // Scale up the image to improve OCR accuracy (2x)
    cv::resize(processed, processed, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);

    return processed;
}

std::string extractText(const cv::Mat& image, const std::string& language) {
    // TODO: Implement OCR functionality - currently disabled due to Tesseract removal
    spdlog::warn("OCR functionality temporarily disabled - Tesseract not available");
    return "OCR_DISABLED";
}

} // namespace detect