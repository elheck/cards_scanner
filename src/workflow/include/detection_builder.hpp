#pragma once

#include <opencv2/opencv.hpp>
#include <scryfall_client.hpp>

#include <filesystem>
#include <optional>

namespace workflow {

enum class CardType {
  modernNormal,
  // Can be extended with more card types in the future
};

class DetectionWorkflow {
public:
  explicit DetectionWorkflow(CardType type);

  // Build and process the card image
  cv::Mat process(const std::filesystem::path &imagePath);

  // Accessors for extracted text (from OCR)
  [[nodiscard]] const std::string &getCardName() const { return cardName_; }
  [[nodiscard]] const std::string &getCollectorNumber() const {
    return collectorNumber_;
  }
  [[nodiscard]] const std::string &getSetName() const { return setName_; }

  // Accessor for enriched card info (from Scryfall)
  [[nodiscard]] const std::optional<api::CardInfo> &getCardInfo() const {
    return cardInfo_;
  }

private:
  CardType type_;

  // Extracted region images
  cv::Mat nameImage_;
  cv::Mat collectorNumberImage_;
  cv::Mat setNameImage_;
  cv::Mat artImage_;

  // Extracted text from regions
  std::string cardName_;
  std::string collectorNumber_;
  std::string setName_;

  // Enriched card info from Scryfall
  std::optional<api::CardInfo> cardInfo_;
  api::ScryfallClient scryfallClient_;

  cv::Mat processModernNormal(const std::filesystem::path &imagePath);
  void readTextFromRegions();
  void lookupCardInfo();
};
} // namespace workflow