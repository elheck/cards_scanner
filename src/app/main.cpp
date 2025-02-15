#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <regex>
#include <tesseract/baseapi.h>

class OCRManager {
public:
  OCRManager(const std::string &lang = "eng") {
    if (api.Init(nullptr, lang.c_str())) {
      throw std::runtime_error("Could not initialize Tesseract");
    }
    api.SetPageSegMode(tesseract::PSM_AUTO);
  }

  std::string extractText(cv::Mat &image) {
    api.SetImage(image.data, image.cols, image.rows, 1, image.step);
    return std::unique_ptr<char[]>(api.GetUTF8Text()).get();
  }

private:
  tesseract::TessBaseAPI api;
};

class ImageProcessor {
public:
  static cv::Mat preprocessImage(const std::string &imagePath) {
    cv::Mat image = cv::imread(imagePath);
    if (image.empty())
      throw std::runtime_error("Could not load image");

    // Crop to bottom-left corner for collector info
    cv::Rect collectorROI(50, image.rows - 150, 300, 100);
    cv::Mat collectorArea = image(collectorROI);

    // Convert to grayscale and threshold
    cv::cvtColor(collectorArea, collectorArea, cv::COLOR_BGR2GRAY);
    cv::threshold(collectorArea, collectorArea, 128, 255,
                  cv::THRESH_BINARY | cv::THRESH_OTSU);

    return collectorArea;
  }

  static cv::Mat preprocessNameArea(const cv::Mat &image) {
    cv::Rect nameROI(100, 100, 400, 50);
    cv::Mat nameArea = image(nameROI);
    cv::cvtColor(nameArea, nameArea, cv::COLOR_BGR2GRAY);
    cv::threshold(nameArea, nameArea, 0, 255,
                  cv::THRESH_BINARY | cv::THRESH_OTSU);
    return nameArea;
  }
};

struct CardInfo {
  std::string name;
  std::string setCode;
  std::string collectorNumber;
  std::string language;
};

class CardDataParser {
public:
  static CardInfo parseOCROutput(const std::string &nameText,
                                 const std::string &collectorText) {
    CardInfo info;
    info.name = cleanName(nameText);

    std::regex pattern(R"((\d+)/(\d+)\s+(\w+)\s+([A-Z0-9]+)\s+([A-Z]{2}))");
    std::smatch matches;

    if (std::regex_search(collectorText, matches, pattern)) {
      info.collectorNumber = matches[1].str();
      info.setCode = matches[4].str();
      info.language = matches[5].str();
    }

    return info;
  }

private:
  static std::string cleanName(const std::string &text) {
    std::string cleaned = text.substr(0, text.find('\n'));
    cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(),
                                 [](unsigned char c) {
                                   return !std::isalnum(c) && !std::isspace(c);
                                 }),
                  cleaned.end());
    return cleaned;
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <image_path>\n";
    return 1;
  }

  try {
    // Process image
    cv::Mat collectorImage = ImageProcessor::preprocessImage(argv[1]);
    cv::Mat fullImage = cv::imread(argv[1]);
    cv::Mat nameImage = ImageProcessor::preprocessNameArea(fullImage);

    // OCR processing
    OCRManager ocr;
    std::string nameText = ocr.extractText(nameImage);
    std::string collectorText = ocr.extractText(collectorImage);

    // Parse results
    CardInfo card = CardDataParser::parseOCROutput(nameText, collectorText);

    // Output results
    std::cout << "Card Name: " << card.name << "\n"
              << "Set Code: " << card.setCode << "\n"
              << "Collector Number: " << card.collectorNumber << "\n"
              << "Language: " << card.language << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
