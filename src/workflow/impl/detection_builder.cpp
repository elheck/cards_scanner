#include <card_detector.hpp>
#include <card_text_ocr.hpp>
#include <detection_builder.hpp>
#include <region_extraction.hpp>
#include <tilt_corrector.hpp>

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace workflow {

DetectionWorkflow::DetectionWorkflow(CardType type) : type_(type) {}

cv::Mat DetectionWorkflow::process(const std::filesystem::path &imagePath) {
  cv::Mat result;
  switch (type_) {
  case CardType::modernNormal:
    result = processModernNormal(imagePath);
    readTextFromRegions();

    break;
  default:
    throw std::runtime_error("Unsupported card type");
  }

  return result; // Return the processed result
}

cv::Mat
DetectionWorkflow::processModernNormal(const std::filesystem::path &imagePath) {
  // Process the card using the detection pipeline
  ASSERT(!imagePath.empty(), "Image path is empty");
  auto card = detect::processCards(imagePath);

  // Apply tilt correction
  card = detect::correctCardTilt(card);

  // Extract bounding boxes
  auto name_box = detect::extractNameRegion(card);
  auto collector_box = detect::extractCollectorNumberRegionModern(card);
  auto set_name_box = detect::extractSetNameRegionModern(card);
  auto art_box = detect::extractArtRegionRegular(card);

  // Store the extracted regions in member variables
  nameImage_ = card(name_box).clone();
  collectorNumberImage_ = card(collector_box).clone();
  setNameImage_ = card(set_name_box).clone();
  artImage_ = card(art_box).clone();

  // Draw all bounding boxes on the card with different colors
  cv::Mat result = card.clone();

  // Name region - Green
  cv::rectangle(result, name_box, cv::Scalar(0, 255, 0), 2);

  // Collector number region - Red
  cv::rectangle(result, collector_box, cv::Scalar(0, 0, 255), 2);

  // Set name region - Blue
  cv::rectangle(result, set_name_box, cv::Scalar(255, 0, 0), 2);

  // Art region - Yellow
  cv::rectangle(result, art_box, cv::Scalar(0, 255, 255), 2);

  return result;
}

void DetectionWorkflow::readTextFromRegions() {
  // Extract text from each region using OCR
  if (!nameImage_.empty()) {
    cardName_ = detect::extractText(nameImage_);
    spdlog::info("Extracted card name: {}", cardName_);
  }

  if (!collectorNumberImage_.empty()) {
    // Use specialized function for digits only
    collectorNumber_ = detect::extractCollectorNumber(collectorNumberImage_);
    spdlog::info("Extracted collector number: {}", collectorNumber_);
  }

  if (!setNameImage_.empty()) {
    // Use specialized function for set code (uppercase letters)
    setName_ = detect::extractSetCode(setNameImage_);
    spdlog::info("Extracted set name: {}", setName_);
  }
}
} // namespace workflow