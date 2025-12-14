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
  int nonZeroCount = cv::countNonZero(processed);
  int totalPixels = processed.rows * processed.cols;
  if (nonZeroCount < totalPixels / 2) {
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
  const char *tessdataPath = TESSDATA_PREFIX;
#else
  const char *tessdataPath = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdataPath, language.c_str()) != 0) {
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
  std::unique_ptr<char[]> outText(tess->GetUTF8Text());
  std::string result = outText ? std::string(outText.get()) : "";

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
    char lastChar = result.back();
    if (!std::isalpha(static_cast<unsigned char>(lastChar)) &&
        lastChar != ' ' && lastChar != '\'') {
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
         std::isalpha(static_cast<unsigned char>(result.back()))) {
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
  int nonZeroCount = cv::countNonZero(processed);
  int totalPixels = processed.rows * processed.cols;
  if (nonZeroCount < totalPixels / 2) {
    cv::bitwise_not(processed, processed);
  }

#ifdef TESSDATA_PREFIX
  const char *tessdataPath = TESSDATA_PREFIX;
#else
  const char *tessdataPath = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdataPath, language.c_str()) != 0) {
    return "";
  }

  tess->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
  // Only allow digits for collector number
  tess->SetVariable("tessedit_char_whitelist", "0123456789");
  tess->SetVariable("load_system_dawg", "0");
  tess->SetVariable("load_freq_dawg", "0");

  tess->SetImage(processed.data, processed.cols, processed.rows, 1,
                 static_cast<int>(processed.step));

  std::unique_ptr<char[]> outText(tess->GetUTF8Text());
  std::string result = outText ? std::string(outText.get()) : "";

  // Keep only digits
  std::string digits;
  for (char c : result) {
    if (c >= '0' && c <= '9') {
      digits += c;
    }
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
  int nonZeroCount = cv::countNonZero(processed);
  int totalPixels = processed.rows * processed.cols;
  if (nonZeroCount < totalPixels / 2) {
    cv::bitwise_not(processed, processed);
  }

#ifdef TESSDATA_PREFIX
  const char *tessdataPath = TESSDATA_PREFIX;
#else
  const char *tessdataPath = nullptr;
#endif
  auto tess = std::make_unique<tesseract::TessBaseAPI>();
  if (tess->Init(tessdataPath, language.c_str()) != 0) {
    return "";
  }

  tess->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
  // Only uppercase letters for set codes
  tess->SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  tess->SetVariable("load_system_dawg", "0");
  tess->SetVariable("load_freq_dawg", "0");

  tess->SetImage(processed.data, processed.cols, processed.rows, 1,
                 static_cast<int>(processed.step));

  std::unique_ptr<char[]> outText(tess->GetUTF8Text());
  std::string result = outText ? std::string(outText.get()) : "";

  // Keep only uppercase letters, limit to exactly 3 chars for set code
  std::string setCode;
  for (char c : result) {
    if (c >= 'A' && c <= 'Z' && setCode.length() < 3) {
      setCode += c;
    }
  }

  tess->End();
  return setCode;
}

} // namespace detect