
#include <detection_builder.hpp>
#include <path_helper.hpp>
#include <pic_helper.hpp>

#include <cxxopts.hpp>
#include <gsl/span>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

[[nodiscard]] std::filesystem::path getCommandLineParameters(int argc,
                                                             char **argv) {
  std::filesystem::path image_path;

  try {
    cxxopts::Options options("card_scanner", "MTG Card Scanner");
    options.add_options()("f,file", "Process a card from an image file",
                          cxxopts::value<std::string>())(
        "h,help", "Show this help message");

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
      spdlog::info("{}", options.help());
      exit(0);
    }

    if (result.count("file") > 0) {
      image_path = result["file"].as<std::string>();
    } else {
      spdlog::critical("Error: No input file specified");
      spdlog::info("{}", options.help());
      abort();
    }
  } catch (const cxxopts::exceptions::exception &e) {
    spdlog::critical("Error parsing options: {}", e.what());
    abort();
  }
  return image_path;
}

int main(int argc, char *argv[]) {

  auto image_path = getCommandLineParameters(argc, argv);

  if (!std::filesystem::exists(image_path)) {
    spdlog::critical("Error: Input file does not exist: {}",
                     image_path.string());
    return 1;
  }

  try {
    // Create a detection builder for modern normal cards
    workflow::DetectionWorkflow builder(workflow::CardType::modernNormal);

    // Process the card using the builder
    auto processed_card = builder.process(image_path);

    if (!misc::saveImage(misc::getTestSamplesPath(), processed_card,
                         "test_out.jpg")) {
      spdlog::critical("Error: Failed to save image");
      return 1;
    }

    spdlog::info("Processing completed successfully");
  } catch (const std::runtime_error &e) {
    spdlog::critical("Error processing card: {}", e.what());
    return 1;
  }

  return 0;
}