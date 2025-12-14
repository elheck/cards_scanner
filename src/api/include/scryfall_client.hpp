#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
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
/// Includes file-based caching to avoid redundant API calls
class ScryfallClient {
public:
  /// Constructor with optional cache directory
  /// @param cacheDir Directory to store cached responses (default:
  /// ~/.cache/mtg_scanner)
  explicit ScryfallClient(const std::filesystem::path &cacheDir = "");
  ~ScryfallClient();

  // Non-copyable
  ScryfallClient(const ScryfallClient &) = delete;
  ScryfallClient &operator=(const ScryfallClient &) = delete;

  /// Look up a card by set code and collector number (most reliable)
  /// Example: getCardByCollectorNumber("dsc", "92")
  [[nodiscard]] std::optional<CardInfo>
  getCardByCollectorNumber(const std::string &setCode,
                           const std::string &collectorNumber);

  /// Fuzzy search for a card by name
  /// Example: getCardByName("Arcane Signet")
  [[nodiscard]] std::optional<CardInfo>
  getCardByFuzzyName(const std::string &name);

  /// Search for cards matching a query (not cached)
  /// Example: searchCards("set:dsc type:artifact")
  [[nodiscard]] static std::vector<CardInfo>
  searchCards(const std::string &query);

  /// Clear all cached data
  void clearCache();

  /// Get cache statistics
  [[nodiscard]] size_t getCacheHits() const { return cacheHits_; }
  [[nodiscard]] size_t getCacheMisses() const { return cacheMisses_; }

private:
  [[nodiscard]] static std::string httpGet(const std::string &url);
  [[nodiscard]] static CardInfo parseCardJson(const std::string &json);
  [[nodiscard]] static std::string urlEncode(const std::string &str);

  // Cache methods
  [[nodiscard]] std::optional<CardInfo> getFromCache(const std::string &key);
  void saveToCache(const std::string &key, const CardInfo &card);
  [[nodiscard]] std::filesystem::path
  getCacheFilePath(const std::string &key) const;
  [[nodiscard]] static std::string cardInfoToJson(const CardInfo &card);

  std::filesystem::path cacheDir_;
  std::unordered_map<std::string, CardInfo> memoryCache_;
  mutable size_t cacheHits_{0};
  mutable size_t cacheMisses_{0};

  static constexpr const char *BASE_URL = "https://api.scryfall.com";
};

} // namespace api
