#include <detection/card_detector.hpp>
#include <detection/tilt_corrector.hpp>
#include <detection/region_extraction.hpp>
#include <misc/pic_helper.hpp>

#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>

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
    std::filesystem::path imagePath;
    ASSERT(argc > 1, "No arguments given", argc);
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
                spdlog::critical("Error: Missing file path");
                printUsage(argv[0]);
                return 1;
            }
        } else {
            spdlog::critical("Error: Unknown option: {}", arg);
            printUsage(argv[0]);
            return 1;
        }
    }

    // Check if image path is provided
    if (imagePath.empty()) {
        spdlog::warn("Error: No input file specified");
        printUsage(argv[0]);
        return 1;
    }

    try {
        // Process the card
        auto processed_card = detect::processCards(imagePath);
        misc::checkImage(processed_card, "Card detection");
        
        // Correct tilt
        processed_card = detect::correctCardTilt(processed_card);
        misc::checkImage(processed_card, "Tilt correction");

        auto region = detect::extractNameRegion(processed_card);
        misc::checkImage(region, "Region extraction");

        //this should change to be handed a root folder by cmake compile definitions
        auto picture_folder = imagePath.parent_path().parent_path() / "sample_out";
        if(!misc::saveImage(picture_folder, region, "test_out.jpg")){
            spdlog::critical("Error: Failed to save image");
            return 1;
        }
    } catch (const std::exception& e) {
        spdlog::critical("Error processing card: {}", e.what());
        return 1;
    }

    return 0;
}