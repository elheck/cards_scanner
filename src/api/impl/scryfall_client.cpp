#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <scryfall_client.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace api {

namespace {
std::string getDefaultCacheDir() {
  const char *home = std::getenv("HOME");
  if (home != nullptr) {
    return std::string(home) + "/.cache/mtg_scanner";
  }
  return "./.mtg_cache";
}
} // namespace

ScryfallClient::ScryfallClient(const std::filesystem::path &cacheDir)
    : cacheDir_(cacheDir.empty() ? std::filesystem::path(getDefaultCacheDir())
                                 : cacheDir) {
  // Create cache directory if it doesn't exist
  if (!std::filesystem::exists(cacheDir_)) {
    std::filesystem::create_directories(cacheDir_);
    spdlog::debug("Created cache directory: {}", cacheDir_.string());
  }
}

ScryfallClient::~ScryfallClient() = default;

std::string ScryfallClient::httpGet(const std::string &url) {
  // Parse URL to extract host and path
  // URL format: https://api.scryfall.com/cards/...
  std::string path = url.substr(std::string(BASE_URL).length());

  httplib::Client cli(BASE_URL);
  cli.set_connection_timeout(10);
  cli.set_read_timeout(10);
  cli.set_follow_location(true);

  httplib::Headers headers = {{"User-Agent", "MTGCardScanner/1.0"}};

  auto res = cli.Get(path, headers);

  if (!res) {
    spdlog::error("HTTP request failed: {}", httplib::to_string(res.error()));
    return "";
  }

  if (res->status != 200) {
    spdlog::debug("Scryfall API returned HTTP {}", res->status);
    if (res->status == 404) {
      return "";
    }
  }

  return res->body;
}

std::string ScryfallClient::urlEncode(const std::string &str) {
  std::ostringstream encoded;
  encoded << std::hex << std::uppercase;

  for (unsigned char c : str) {
    if (std::isalnum(c) != 0 || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else if (c == ' ') {
      encoded << '+';
    } else {
      encoded << '%' << std::setw(2) << std::setfill('0')
              << static_cast<int>(c);
    }
  }

  return encoded.str();
}

CardInfo ScryfallClient::parseCardJson(const std::string &json) {
  CardInfo card;

  try {
    auto j = nlohmann::json::parse(json);

    if (j.contains("object") && j["object"] == "error") {
      spdlog::debug("Scryfall API error: {}",
                    j.value("details", "Unknown error"));
      return card;
    }

    card.id = j.value("id", "");
    card.name = j.value("name", "");
    card.setCode = j.value("set", "");
    card.setName = j.value("set_name", "");
    card.collectorNumber = j.value("collector_number", "");
    card.rarity = j.value("rarity", "");
    card.typeLine = j.value("type_line", "");
    card.manaCost = j.value("mana_cost", "");
    card.oracleText = j.value("oracle_text", "");

    // Get image URI (prefer normal size)
    if (j.contains("image_uris") && j["image_uris"].contains("normal")) {
      card.imageUri = j["image_uris"]["normal"];
    } else if (j.contains("card_faces") && !j["card_faces"].empty()) {
      // Double-faced cards have images in card_faces
      auto &face = j["card_faces"][0];
      if (face.contains("image_uris") &&
          face["image_uris"].contains("normal")) {
        card.imageUri = face["image_uris"]["normal"];
      }
    }

    // Get prices
    if (j.contains("prices")) {
      auto &prices = j["prices"];
      if (prices.contains("usd") && !prices["usd"].is_null()) {
        try {
          card.priceUsd = std::stod(prices["usd"].get<std::string>());
        } catch (const std::exception &e) {
          spdlog::debug("Failed to parse USD price: {}", e.what());
        }
      }
      if (prices.contains("eur") && !prices["eur"].is_null()) {
        try {
          card.priceEur = std::stod(prices["eur"].get<std::string>());
        } catch (const std::exception &e) {
          spdlog::debug("Failed to parse EUR price: {}", e.what());
        }
      }
    }

    card.isValid = !card.id.empty() && !card.name.empty();

  } catch (const nlohmann::json::exception &e) {
    spdlog::error("Failed to parse Scryfall response: {}", e.what());
  }

  return card;
}

std::optional<CardInfo>
ScryfallClient::getCardByCollectorNumber(const std::string &setCode,
                                         const std::string &collectorNumber) {

  if (setCode.empty() || collectorNumber.empty()) {
    return std::nullopt;
  }

  // Convert set code to lowercase (Scryfall requirement)
  std::string lower_set_code = setCode;
  std::transform(lower_set_code.begin(), lower_set_code.end(),
                 lower_set_code.begin(), ::tolower);

  // Check cache first
  std::string cache_key = "collector_" + lower_set_code + "_" + collectorNumber;
  if (auto cached = getFromCache(cache_key)) {
    spdlog::debug("Cache hit for {}/{}", lower_set_code, collectorNumber);
    ++cacheHits_;
    return cached;
  }
  ++cacheMisses_;

  std::string url = std::string(BASE_URL) + "/cards/" +
                    urlEncode(lower_set_code) + "/" +
                    urlEncode(collectorNumber);

  spdlog::debug("Scryfall lookup: {}", url);

  std::string response = httpGet(url);
  if (response.empty()) {
    return std::nullopt;
  }

  CardInfo card = parseCardJson(response);
  if (card.isValid) {
    saveToCache(cache_key, card);
    spdlog::info("Found card: {} ({} #{})", card.name, card.setCode,
                 card.collectorNumber);
    return card;
  }

  return std::nullopt;
}

std::optional<CardInfo>
ScryfallClient::getCardByFuzzyName(const std::string &name) {

  if (name.empty()) {
    return std::nullopt;
  }

  // Normalize name for cache key (lowercase, no special chars)
  std::string normalized_name = name;
  std::transform(normalized_name.begin(), normalized_name.end(),
                 normalized_name.begin(), ::tolower);
  std::string cache_key = "name_" + normalized_name;

  // Check cache first
  if (auto cached = getFromCache(cache_key)) {
    spdlog::debug("Cache hit for name: {}", name);
    ++cacheHits_;
    return cached;
  }
  ++cacheMisses_;

  std::string url =
      std::string(BASE_URL) + "/cards/named?fuzzy=" + urlEncode(name);

  spdlog::debug("Scryfall fuzzy search: {}", url);

  std::string response = httpGet(url);
  if (response.empty()) {
    return std::nullopt;
  }

  CardInfo card = parseCardJson(response);
  if (card.isValid) {
    saveToCache(cache_key, card);
    spdlog::info("Found card by name: {} ({} #{})", card.name, card.setCode,
                 card.collectorNumber);
    return card;
  }

  return std::nullopt;
}

std::vector<CardInfo> ScryfallClient::searchCards(const std::string &query) {
  std::vector<CardInfo> results;

  if (query.empty()) {
    return results;
  }

  std::string url =
      std::string(BASE_URL) + "/cards/search?q=" + urlEncode(query);

  spdlog::debug("Scryfall search: {}", url);

  std::string response = httpGet(url);
  if (response.empty()) {
    return results;
  }

  try {
    auto j = nlohmann::json::parse(response);

    if (j.contains("object") && j["object"] == "list" && j.contains("data")) {
      for (const auto &card_json : j["data"]) {
        CardInfo card = parseCardJson(card_json.dump());
        if (card.isValid) {
          results.push_back(card);
        }
      }
    }
  } catch (const nlohmann::json::exception &e) {
    spdlog::error("Failed to parse search results: {}", e.what());
  }

  return results;
}

// Cache implementation

std::filesystem::path
ScryfallClient::getCacheFilePath(const std::string &key) const {
  return cacheDir_ / (key + ".json");
}

std::string ScryfallClient::cardInfoToJson(const CardInfo &card) {
  nlohmann::json j;
  j["id"] = card.id;
  j["name"] = card.name;
  j["set"] = card.setCode;
  j["set_name"] = card.setName;
  j["collector_number"] = card.collectorNumber;
  j["rarity"] = card.rarity;
  j["type_line"] = card.typeLine;
  j["mana_cost"] = card.manaCost;
  j["oracle_text"] = card.oracleText;
  j["image_uris"]["normal"] = card.imageUri;
  j["prices"]["usd"] = card.priceUsd > 0
                           ? nlohmann::json(std::to_string(card.priceUsd))
                           : nlohmann::json(nullptr);
  j["prices"]["eur"] = card.priceEur > 0
                           ? nlohmann::json(std::to_string(card.priceEur))
                           : nlohmann::json(nullptr);
  return j.dump(2);
}

std::optional<CardInfo> ScryfallClient::getFromCache(const std::string &key) {
  // Check memory cache first
  auto it = memoryCache_.find(key);
  if (it != memoryCache_.end()) {
    return it->second;
  }

  // Check file cache
  std::filesystem::path cache_path = getCacheFilePath(key);
  if (!std::filesystem::exists(cache_path)) {
    return std::nullopt;
  }

  try {
    std::ifstream file(cache_path);
    if (!file.is_open()) {
      return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();

    CardInfo card = parseCardJson(json);
    if (card.isValid) {
      // Store in memory cache for faster subsequent access
      memoryCache_[key] = card;
      return card;
    }
  } catch (const std::exception &e) {
    spdlog::debug("Failed to read cache file {}: {}", cache_path.string(),
                  e.what());
  }

  return std::nullopt;
}

void ScryfallClient::saveToCache(const std::string &key, const CardInfo &card) {
  // Save to memory cache
  memoryCache_[key] = card;

  // Save to file cache
  std::filesystem::path cache_path = getCacheFilePath(key);
  try {
    std::ofstream file(cache_path);
    if (file.is_open()) {
      file << cardInfoToJson(card);
      spdlog::debug("Cached card to {}", cache_path.string());
    }
  } catch (const std::exception &e) {
    spdlog::warn("Failed to write cache file {}: {}", cache_path.string(),
                 e.what());
  }
}

void ScryfallClient::clearCache() {
  memoryCache_.clear();
  cacheHits_ = 0;
  cacheMisses_ = 0;

  if (std::filesystem::exists(cacheDir_)) {
    for (const auto &entry : std::filesystem::directory_iterator(cacheDir_)) {
      if (entry.path().extension() == ".json") {
        std::filesystem::remove(entry.path());
      }
    }
    spdlog::info("Cache cleared");
  }
}

} // namespace api
