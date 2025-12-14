#include <card_text_ocr.hpp>
#include <leptonica/allheaders.h>
#include <spdlog/spdlog.h>
#include <tesseract/baseapi.h>

#include <memory>

namespace detect {

cv::Mat preprocessForOcr(const cv::Mat &image) {
  cv::Mat processed;

  // Convert to grayscale if the image is in color
  if (image.channels() == 3) {
    cv::cvtColor(image, processed, cv::COLOR_BGR2GRAY);
  } else {
    processed = image.clone();
  }

  // Scale up first for better detail preservation (3x for small text regions)
  cv::resize(processed, processed, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);

  // Apply bilateral filter to reduce noise while preserving edges
  cv::Mat filtered;
  cv::bilateralFilter(processed, filtered, 9, 75, 75);

  // Use Otsu's thresholding for better automatic threshold selection
  cv::threshold(filtered, processed, 0, 255,
                cv::THRESH_BINARY | cv::THRESH_OTSU);

  // Invert if the text appears to be light on dark background
  // (check if more than 50% of pixels are dark)
  int non_zero_count = cv::countNonZero(processed);
  int total_pixels = processed.rows * processed.cols;
  if (non_zero_count < total_pixels / 2) {
    cv::bitwise_not(processed, processed);
  }

  // Light morphological operations to clean up noise
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
  cv::morphologyEx(processed, processed, cv::MORPH_CLOSE, kernel);

  return processed;
}

std::string extractText(const cv::Mat &image, const std::string &language) {
  if (image.empty()) {
    spdlog::error("Cannot extract text from empty image");
    return "";
  }

  // Preprocess the image for better OCR results
  cv::Mat processed = preprocessForOcr(image);

  // Initialize Tesseract with tessdata path from build configuration
#ifdef TESSDATA_PREFIX
  const char *tessdata_path = TESSDATA_PREFIX;
#else
  const char *tessdata_path = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdata_path, language.c_str()) != 0) {
    spdlog::error("Failed to initialize Tesseract with language: {}", language);
    return "";
  }

  // Set page segmentation mode to single line for card text regions
  tess->SetPageSegMode(tesseract::PSM_SINGLE_LINE);

  // Configure Tesseract for better accuracy on stylized text
  tess->SetVariable(
      "tessedit_char_whitelist",
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 '-,.");
  tess->SetVariable("load_system_dawg", "0");
  tess->SetVariable("load_freq_dawg", "0");

  // Set the image data
  tess->SetImage(processed.data, processed.cols, processed.rows, 1,
                 static_cast<int>(processed.step));

  // Extract text
  std::unique_ptr<char, decltype(&std::free)> out_text(tess->GetUTF8Text(),
                                                       &std::free);
  std::string result = out_text ? std::string(out_text.get()) : "";

  // Trim whitespace
  auto start = result.find_first_not_of(" \t\n\r");
  auto end = result.find_last_not_of(" \t\n\r");
  if (start != std::string::npos && end != std::string::npos) {
    result = result.substr(start, end - start + 1);
  } else {
    result.clear();
  }

  // Remove trailing non-letter/non-space characters (artifacts like "- j j")
  while (!result.empty()) {
    char last_char = result.back();
    if (std::isalpha(static_cast<unsigned char>(last_char)) == 0 &&
        last_char != ' ' && last_char != '\'') {
      result.pop_back();
    } else {
      break;
    }
  }
  // Trim trailing spaces after removing artifacts
  while (!result.empty() && result.back() == ' ') {
    result.pop_back();
  }
  // Remove trailing single characters (likely noise)
  while (result.size() >= 2 && result[result.size() - 2] == ' ' &&
         std::isalpha(static_cast<unsigned char>(result.back())) != 0) {
    result.pop_back();
    result.pop_back();
  }
  // Final trim
  while (!result.empty() && result.back() == ' ') {
    result.pop_back();
  }

  tess->End();
  return result;
}

std::string extractCollectorNumber(const cv::Mat &image,
                                   const std::string &language) {
  if (image.empty()) {
    return "";
  }

  cv::Mat processed;

  // Convert to grayscale
  if (image.channels() == 3) {
    cv::cvtColor(image, processed, cv::COLOR_BGR2GRAY);
  } else {
    processed = image.clone();
  }

  // Scale up for better digit recognition
  cv::resize(processed, processed, cv::Size(), 4.0, 4.0, cv::INTER_CUBIC);

  // Bilateral filter to preserve edges
  cv::Mat filtered;
  cv::bilateralFilter(processed, filtered, 9, 75, 75);

  // Otsu's thresholding
  cv::threshold(filtered, processed, 0, 255,
                cv::THRESH_BINARY | cv::THRESH_OTSU);

  // Auto-invert if needed
  int non_zero_count = cv::countNonZero(processed);
  int total_pixels = processed.rows * processed.cols;
  if (non_zero_count < total_pixels / 2) {
    cv::bitwise_not(processed, processed);
  }

#ifdef TESSDATA_PREFIX
  const char *tessdata_path = TESSDATA_PREFIX;
#else
  const char *tessdata_path = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdata_path, language.c_str()) != 0) {
    return "";
  }

  tess->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
  // Only allow digits for collector number
  tess->SetVariable("tessedit_char_whitelist", "0123456789");
  tess->SetVariable("load_system_dawg", "0");
  tess->SetVariable("load_freq_dawg", "0");

  tess->SetImage(processed.data, processed.cols, processed.rows, 1,
                 static_cast<int>(processed.step));

  std::unique_ptr<char, decltype(&std::free)> out_text(tess->GetUTF8Text(),
                                                       &std::free);
  std::string result = out_text ? std::string(out_text.get()) : "";

  // Keep only digits
  std::string digits;
  for (char c : result) {
    if (c >= '0' && c <= '9') {
      digits += c;
    }
  }

  // Collector numbers are exactly 3 digits (take last 3)
  if (digits.length() > 3) {
    digits = digits.substr(digits.length() - 3);
  }

  // Remove leading zeros (Scryfall uses numbers without leading zeros)
  size_t first_non_zero = digits.find_first_not_of('0');
  if (first_non_zero != std::string::npos) {
    digits = digits.substr(first_non_zero);
  } else if (!digits.empty()) {
    digits = "0"; // Handle "000" case
  }

  tess->End();
  return digits;
}

std::string extractSetCode(const cv::Mat &image, const std::string &language) {
  if (image.empty()) {
    return "";
  }

  cv::Mat processed;

  // Convert to grayscale
  if (image.channels() == 3) {
    cv::cvtColor(image, processed, cv::COLOR_BGR2GRAY);
  } else {
    processed = image.clone();
  }

  // Scale up significantly for small text
  cv::resize(processed, processed, cv::Size(), 5.0, 5.0, cv::INTER_LANCZOS4);

  // Simple bilateral filter to reduce noise while preserving edges
  cv::Mat filtered;
  cv::bilateralFilter(processed, filtered, 9, 75, 75);

  // Otsu's thresholding
  cv::threshold(filtered, processed, 0, 255,
                cv::THRESH_BINARY | cv::THRESH_OTSU);

  // Auto-invert if needed (text should be dark on light)
  int non_zero_count = cv::countNonZero(processed);
  int total_pixels = processed.rows * processed.cols;
  if (non_zero_count < total_pixels / 2) {
    cv::bitwise_not(processed, processed);
  }

#ifdef TESSDATA_PREFIX
  const char *tessdata_path = TESSDATA_PREFIX;
#else
  const char *tessdata_path = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdata_path, language.c_str()) != 0) {
    return "";
  }

  tess->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
  // Only uppercase letters for set codes
  tess->SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  tess->SetVariable("load_system_dawg", "0");
  tess->SetVariable("load_freq_dawg", "0");

  tess->SetImage(processed.data, processed.cols, processed.rows, 1,
                 static_cast<int>(processed.step));

  std::unique_ptr<char, decltype(&std::free)> out_text(tess->GetUTF8Text(),
                                                       &std::free);
  std::string result = out_text ? std::string(out_text.get()) : "";

  // Keep only uppercase letters, limit to exactly 3 chars for set code
  std::string set_code;
  for (char c : result) {
    if (c >= 'A' && c <= 'Z' && set_code.length() < 3) {
      set_code += c;
    }
  }

  tess->End();
  return set_code;
}

} // namespace detect