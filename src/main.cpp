#include "card_processor.hpp"
#include <iostream>
#include <string>

void printUsage(const char *programName) {
  std::cout << "Usage: " << programName << " [options]" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -f, --file <path>    Process a card from an image file"
            << std::endl;
  std::cout << "  -h, --help           Show this help message" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string imagePath;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "-f" || arg == "--file") {
      if (i + 1 < argc) {
        imagePath = argv[++i];
      } else {
        std::cerr << "Error: Missing file path" << std::endl;
        printUsage(argv[0]);
        return 1;
      }
    } else {
      std::cerr << "Error: Unknown option: " << arg << std::endl;
      printUsage(argv[0]);
      return 1;
    }
  }

  // Create a card processor
  CardProcessor processor;

  // Check if image path is provided
  if (imagePath.empty()) {
    std::cerr << "Error: No input file specified" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // Load the image
  if (!processor.loadImage(imagePath)) {
    std::cerr << "Error: Failed to load image" << std::endl;
    return 1;
  }

  // Process the card
  if (!processor.processCard()) {
    std::cerr << "Error: Failed to process card" << std::endl;
    return 1;
  }

  // Display the images
  processor.displayImages();

  return 0;
}