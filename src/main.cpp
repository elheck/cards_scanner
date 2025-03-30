#include <workflow/detection_builder.hpp>
#include <misc/pic_helper.hpp>
#include <misc/path_helper.hpp>

#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>
#include <gsl/span>

#include <iostream>
#include <string>

void printUsage(std::string_view programName) {
    spdlog::info("Usage: {} [options]", programName);
    spdlog::info("Options:");
    spdlog::info("  -f, --file <path>    Process a card from an image file");
    spdlog::info("  -h, --help           Show this help message");
}

int main(int argc, char *argv[]) {
    const gsl::span args{argv, static_cast<std::size_t>(argc)};
    std::filesystem::path imagePath;
    
    ASSERT(args.size() > 1, "No arguments given", args.size());

    // Parse command line arguments
    for (size_t i = 1; i < args.size(); ++i) {
        std::string_view arg{args[i]};

        if (arg == "-h" || arg == "--help") {
            printUsage(args[0]);
            return 0;
        } else if (arg == "-f" || arg == "--file") {
            if (i + 1 < args.size()) {
                imagePath = args[++i];
            } else {
                spdlog::critical("Error: Missing file path");
                printUsage(args[0]);
                return 1;
            }
        } else {
            spdlog::critical("Error: Unknown option: {}", arg);
            printUsage(args[0]);
            return 1;
        }
    }

    // Check if image path is provided
    if (imagePath.empty()) {
        spdlog::warn("Error: No input file specified");
        printUsage(args[0]);
        return 1;
    }

    try {
        // Create a detection builder for modern normal cards
        workflow::DetectionBuilder builder(workflow::CardType::modernNormal);
        
        // Process the card using the builder
        auto processed_card = builder.process(imagePath);

        if(!misc::saveImage(misc::getTestSamplesPath(), processed_card, "test_out.jpg")){
            spdlog::critical("Error: Failed to save image");
            return 1;
        }
    } catch (const std::exception& e) {
        spdlog::critical("Error processing card: {}", e.what());
        return 1;
    }

    return 0;
}