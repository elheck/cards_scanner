#include <curl/curl.h>
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
// Callback for libcurl to write received data
size_t writeCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  size_t totalSize = size * nmemb;
  userp->append(static_cast<char *>(contents), totalSize);
  return totalSize;
}

std::string getDefaultCacheDir() {
  const char* home = std::getenv("HOME");
  if (home) {
    return std::string(home) + "/.cache/mtg_scanner";
  }
  return "./.mtg_cache";
}
} // namespace

ScryfallClient::ScryfallClient(const std::filesystem::path& cacheDir) 
    : cacheDir_(cacheDir.empty() ? std::filesystem::path(getDefaultCacheDir()) : cacheDir) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  
  // Create cache directory if it doesn't exist
  if (!std::filesystem::exists(cacheDir_)) {
    std::filesystem::create_directories(cacheDir_);
    spdlog::debug("Created cache directory: {}", cacheDir_.string());
  }
}

ScryfallClient::~ScryfallClient() { curl_global_cleanup(); }

std::string ScryfallClient::httpGet(const std::string &url) const {
  CURL *curl = curl_easy_init();
  if (!curl) {
    spdlog::error("Failed to initialize CURL");
    return "";
  }

  std::string response;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "MTGCardScanner/1.0");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    spdlog::error("CURL request failed: {}", curl_easy_strerror(res));
    response.clear();
  }

  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (httpCode != 200) {
    spdlog::debug("Scryfall API returned HTTP {}", httpCode);
    if (httpCode == 404) {
      response.clear();
    }
  }

  curl_easy_cleanup(curl);
  return response;
}

std::string ScryfallClient::urlEncode(const std::string &str) const {
  std::ostringstream encoded;
  encoded << std::hex << std::uppercase;

  for (unsigned char c : str) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
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

CardInfo ScryfallClient::parseCardJson(const std::string &json) const {
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
        } catch (...) {
        }
      }
      if (prices.contains("eur") && !prices["eur"].is_null()) {
        try {
          card.priceEur = std::stod(prices["eur"].get<std::string>());
        } catch (...) {
        }
      }
    }

    card.isValid = !card.id.empty() && !card.name.empty();

  } catch (const nlohmann::json::exception &e) {
    spdlog::error("Failed to parse Scryfall response: {}", e.what());
  }

  return card;
}

std::optional<CardInfo> ScryfallClient::getCardByCollectorNumber(
    const std::string &setCode, const std::string &collectorNumber) {

  if (setCode.empty() || collectorNumber.empty()) {
    return std::nullopt;
  }

  // Convert set code to lowercase (Scryfall requirement)
  std::string lowerSetCode = setCode;
  std::transform(lowerSetCode.begin(), lowerSetCode.end(), lowerSetCode.begin(),
                 ::tolower);

  // Check cache first
  std::string cacheKey = "collector_" + lowerSetCode + "_" + collectorNumber;
  if (auto cached = getFromCache(cacheKey)) {
    spdlog::debug("Cache hit for {}/{}", lowerSetCode, collectorNumber);
    ++cacheHits_;
    return cached;
  }
  ++cacheMisses_;

  std::string url = std::string(BASE_URL) + "/cards/" +
                    urlEncode(lowerSetCode) + "/" + urlEncode(collectorNumber);

  spdlog::debug("Scryfall lookup: {}", url);

  std::string response = httpGet(url);
  if (response.empty()) {
    return std::nullopt;
  }

  CardInfo card = parseCardJson(response);
  if (card.isValid) {
    saveToCache(cacheKey, card);
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
  std::string normalizedName = name;
  std::transform(normalizedName.begin(), normalizedName.end(), 
                 normalizedName.begin(), ::tolower);
  std::string cacheKey = "name_" + normalizedName;

  // Check cache first
  if (auto cached = getFromCache(cacheKey)) {
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
    saveToCache(cacheKey, card);
    spdlog::info("Found card by name: {} ({} #{})", card.name, card.setCode,
                 card.collectorNumber);
    return card;
  }

  return std::nullopt;
}

std::vector<CardInfo>
ScryfallClient::searchCards(const std::string &query) const {
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
      for (const auto &cardJson : j["data"]) {
        CardInfo card = parseCardJson(cardJson.dump());
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

std::filesystem::path ScryfallClient::getCacheFilePath(const std::string& key) const {
  return cacheDir_ / (key + ".json");
}

std::string ScryfallClient::cardInfoToJson(const CardInfo& card) const {
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
  j["prices"]["usd"] = card.priceUsd > 0 ? std::to_string(card.priceUsd) : nullptr;
  j["prices"]["eur"] = card.priceEur > 0 ? std::to_string(card.priceEur) : nullptr;
  return j.dump(2);
}

std::optional<CardInfo> ScryfallClient::getFromCache(const std::string& key) {
  // Check memory cache first
  auto it = memoryCache_.find(key);
  if (it != memoryCache_.end()) {
    return it->second;
  }

  // Check file cache
  std::filesystem::path cachePath = getCacheFilePath(key);
  if (!std::filesystem::exists(cachePath)) {
    return std::nullopt;
  }

  try {
    std::ifstream file(cachePath);
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
  } catch (const std::exception& e) {
    spdlog::debug("Failed to read cache file {}: {}", cachePath.string(), e.what());
  }

  return std::nullopt;
}

void ScryfallClient::saveToCache(const std::string& key, const CardInfo& card) {
  // Save to memory cache
  memoryCache_[key] = card;

  // Save to file cache
  std::filesystem::path cachePath = getCacheFilePath(key);
  try {
    std::ofstream file(cachePath);
    if (file.is_open()) {
      file << cardInfoToJson(card);
      spdlog::debug("Cached card to {}", cachePath.string());
    }
  } catch (const std::exception& e) {
    spdlog::warn("Failed to write cache file {}: {}", cachePath.string(), e.what());
  }
}

void ScryfallClient::clearCache() {
  memoryCache_.clear();
  cacheHits_ = 0;
  cacheMisses_ = 0;

  if (std::filesystem::exists(cacheDir_)) {
    for (const auto& entry : std::filesystem::directory_iterator(cacheDir_)) {
      if (entry.path().extension() == ".json") {
        std::filesystem::remove(entry.path());
      }
    }
    spdlog::info("Cache cleared");
  }
}

} // namespace api
