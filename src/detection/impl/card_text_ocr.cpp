#include <detection/card_text_ocr.hpp>
#include <tesseract/baseapi.h>
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
    // Initialize Tesseract OCR
    tesseract::TessBaseAPI tess;
    if (tess.Init(nullptr, "eng")) {
        spdlog::error("Could not initialize tesseract.");
        return "";
    }

    // Preprocess the image
    cv::Mat processed = preprocessForOcr(image);

    // Set image data
    tess.SetImage(processed.data, processed.cols, processed.rows,
                  processed.channels(), processed.step);

    // Set OCR parameters suitable for card text
    tess.SetPageSegMode(tesseract::PSM_SINGLE_LINE); // Assume text is in a single line
    tess.SetVariable("tessedit_char_whitelist", 
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-',."); 

    // Get OCR result
    std::string result = {tess.GetUTF8Text()};
    // Clean up
    tess.End();

    // Trim whitespace
    result.erase(0, result.find_first_not_of(" \n\r\t"));
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result;
}

} // namespace detect