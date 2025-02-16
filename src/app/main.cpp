#include <core/imageProcessor.hpp>
#include <opencv2/opencv.hpp>
#include <regex>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <image_path>\n";
    return 1;
  }

  try {
    // Process image
    cv::Mat fullImage = cv::imread(argv[1]);
    ImageProcessor processor(fullImage);
    processor.processCard();
    processor.displayCardParts();

    // OCR processing

    // Parse results

    // Output results

  } catch (std::runtime_error &e) {
    throw e;
  }
  return 0;
}
