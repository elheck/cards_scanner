#pragma once

#include <optional>
#include <string>
#include <vector>

namespace api {

/// Card information returned from Scryfall API
struct CardInfo {
  std::string id;      // Scryfall UUID
  std::string name;    // Card name
  std::string setCode; // Set code (e.g., "DSC")
  std::string
      setName; // Full set name (e.g., "Duskmourn: House of Horror Commander")
  std::string collectorNumber; // Collector number
  std::string rarity;          // common, uncommon, rare, mythic
  std::string typeLine;        // Type line (e.g., "Artifact")
  std::string manaCost;        // Mana cost (e.g., "{2}")
  std::string oracleText;      // Card rules text
  std::string imageUri;        // URI to card image
  double priceUsd{0.0};        // USD price
  double priceEur{0.0};        // EUR price
  bool isValid{false};         // Whether this card info is valid
};

/// Client for the Scryfall API (https://scryfall.com/docs/api)
class ScryfallClient {
public:
  ScryfallClient();
  ~ScryfallClient();

  // Non-copyable
  ScryfallClient(const ScryfallClient &) = delete;
  ScryfallClient &operator=(const ScryfallClient &) = delete;

  /// Look up a card by set code and collector number (most reliable)
  /// Example: getCardByCollectorNumber("dsc", "0092")
  [[nodiscard]] std::optional<CardInfo>
  getCardByCollectorNumber(const std::string &setCode,
                           const std::string &collectorNumber) const;

  /// Fuzzy search for a card by name
  /// Example: getCardByName("Arcane Signet")
  [[nodiscard]] std::optional<CardInfo>
  getCardByFuzzyName(const std::string &name) const;

  /// Search for cards matching a query
  /// Example: searchCards("set:dsc type:artifact")
  [[nodiscard]] std::vector<CardInfo>
  searchCards(const std::string &query) const;

private:
  [[nodiscard]] std::string httpGet(const std::string &url) const;
  [[nodiscard]] CardInfo parseCardJson(const std::string &json) const;
  [[nodiscard]] std::string urlEncode(const std::string &str) const;

  static constexpr const char *BASE_URL = "https://api.scryfall.com";
};

} // namespace api
