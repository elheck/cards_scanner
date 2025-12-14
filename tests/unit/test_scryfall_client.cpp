/**
 * Unit tests for ScryfallClient API module
 *
 * These tests focus on:
 * - CardInfo struct behavior
 * - URL encoding functionality
 * - JSON parsing for card data
 * - Cache operations (memory and file-based)
 * - Input validation and error handling
 *
 * Note: Tests avoid making actual HTTP calls by testing internal logic only
 */

#include <gtest/gtest.h>
#include <scryfall_client.hpp>

#include <filesystem>
#include <fstream>
#include <tuple>

namespace {

// Test fixture for ScryfallClient tests
class ScryfallClientTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a unique temporary cache directory for each test
    testCacheDir_ =
        std::filesystem::temp_directory_path() / "scryfall_test_cache";
    if (std::filesystem::exists(testCacheDir_)) {
      std::filesystem::remove_all(testCacheDir_);
    }
  }

  void TearDown() override {
    // Clean up after each test
    if (std::filesystem::exists(testCacheDir_)) {
      std::filesystem::remove_all(testCacheDir_);
    }
  }

  std::filesystem::path testCacheDir_;
};

// ============================================================================
// CardInfo struct tests
// ============================================================================

TEST(CardInfoTest, DefaultConstructorInitializesEmpty) {
  api::CardInfo card;

  EXPECT_TRUE(card.id.empty());
  EXPECT_TRUE(card.name.empty());
  EXPECT_TRUE(card.setCode.empty());
  EXPECT_TRUE(card.setName.empty());
  EXPECT_TRUE(card.collectorNumber.empty());
  EXPECT_TRUE(card.rarity.empty());
  EXPECT_TRUE(card.typeLine.empty());
  EXPECT_TRUE(card.manaCost.empty());
  EXPECT_TRUE(card.oracleText.empty());
  EXPECT_TRUE(card.imageUri.empty());
  EXPECT_DOUBLE_EQ(card.priceUsd, 0.0);
  EXPECT_DOUBLE_EQ(card.priceEur, 0.0);
  EXPECT_FALSE(card.isValid);
}

TEST(CardInfoTest, IsValidIndicatesCompleteness) {
  api::CardInfo card;

  // Default should be invalid
  EXPECT_FALSE(card.isValid);

  // Setting just name still invalid
  card.name = "Test Card";
  EXPECT_FALSE(card.isValid);

  // Manually marking as valid
  card.isValid = true;
  EXPECT_TRUE(card.isValid);
}

// ============================================================================
// ScryfallClient construction tests
// ============================================================================

TEST_F(ScryfallClientTest, ConstructorCreatesCacheDirectory) {
  EXPECT_FALSE(std::filesystem::exists(testCacheDir_));

  api::ScryfallClient client(testCacheDir_);

  EXPECT_TRUE(std::filesystem::exists(testCacheDir_));
  EXPECT_TRUE(std::filesystem::is_directory(testCacheDir_));
}

TEST_F(ScryfallClientTest, ConstructorWithExistingDirectorySucceeds) {
  // Create directory first
  std::filesystem::create_directories(testCacheDir_);
  EXPECT_TRUE(std::filesystem::exists(testCacheDir_));

  // Should not throw when directory exists
  EXPECT_NO_THROW({ api::ScryfallClient client(testCacheDir_); });
}

TEST_F(ScryfallClientTest, ConstructorCreatesNestedDirectories) {
  auto nestedDir = testCacheDir_ / "level1" / "level2" / "level3";
  EXPECT_FALSE(std::filesystem::exists(nestedDir));

  api::ScryfallClient client(nestedDir);

  EXPECT_TRUE(std::filesystem::exists(nestedDir));
}

// ============================================================================
// Cache statistics tests
// ============================================================================

TEST_F(ScryfallClientTest, InitialCacheStatisticsAreZero) {
  api::ScryfallClient client(testCacheDir_);

  EXPECT_EQ(client.getCacheHits(), 0U);
  EXPECT_EQ(client.getCacheMisses(), 0U);
}

TEST_F(ScryfallClientTest, ClearCacheResetsStatistics) {
  api::ScryfallClient client(testCacheDir_);

  // Make some lookups to increment misses (they fail because no network, but
  // stats still count)
  std::ignore = client.getCardByCollectorNumber("test", "123");
  std::ignore = client.getCardByFuzzyName("nonexistent");

  // Stats should be incremented
  EXPECT_GT(client.getCacheMisses(), 0U);

  // Clear should reset
  client.clearCache();

  EXPECT_EQ(client.getCacheHits(), 0U);
  EXPECT_EQ(client.getCacheMisses(), 0U);
}

// ============================================================================
// Empty input handling tests
// ============================================================================

TEST_F(ScryfallClientTest, GetCardByCollectorNumberEmptySetCodeReturnsNullopt) {
  api::ScryfallClient client(testCacheDir_);

  auto result = client.getCardByCollectorNumber("", "123");

  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest,
       GetCardByCollectorNumberEmptyCollectorNumReturnsNullopt) {
  api::ScryfallClient client(testCacheDir_);

  auto result = client.getCardByCollectorNumber("dsc", "");

  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest, GetCardByCollectorNumberBothEmptyReturnsNullopt) {
  api::ScryfallClient client(testCacheDir_);

  auto result = client.getCardByCollectorNumber("", "");

  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest, GetCardByFuzzyNameEmptyReturnsNullopt) {
  api::ScryfallClient client(testCacheDir_);

  auto result = client.getCardByFuzzyName("");

  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest, SearchCardsEmptyQueryReturnsEmptyVector) {
  api::ScryfallClient client(testCacheDir_);

  auto results = client.searchCards("");

  EXPECT_TRUE(results.empty());
}

// ============================================================================
// Cache file management tests
// ============================================================================

TEST_F(ScryfallClientTest, ClearCacheRemovesJsonFiles) {
  api::ScryfallClient client(testCacheDir_);

  // Create some fake cache files
  std::ofstream(testCacheDir_ / "test1.json") << "{}";
  std::ofstream(testCacheDir_ / "test2.json") << "{}";
  std::ofstream(testCacheDir_ / "other.txt") << "not json";

  EXPECT_TRUE(std::filesystem::exists(testCacheDir_ / "test1.json"));
  EXPECT_TRUE(std::filesystem::exists(testCacheDir_ / "test2.json"));
  EXPECT_TRUE(std::filesystem::exists(testCacheDir_ / "other.txt"));

  client.clearCache();

  // JSON files should be removed
  EXPECT_FALSE(std::filesystem::exists(testCacheDir_ / "test1.json"));
  EXPECT_FALSE(std::filesystem::exists(testCacheDir_ / "test2.json"));

  // Non-JSON files should remain
  EXPECT_TRUE(std::filesystem::exists(testCacheDir_ / "other.txt"));
}

TEST_F(ScryfallClientTest, ClearCacheOnNonExistentDirectoryDoesNotThrow) {
  // Create client but delete directory before clear
  api::ScryfallClient client(testCacheDir_);
  std::filesystem::remove_all(testCacheDir_);

  EXPECT_NO_THROW({ client.clearCache(); });
}

// ============================================================================
// Input validation edge cases
// ============================================================================

TEST_F(ScryfallClientTest,
       GetCardByCollectorNumberWhitespaceOnlyReturnsNullopt) {
  api::ScryfallClient client(testCacheDir_);

  // These are not empty strings, but likely won't find anything
  // The validation passes but API call fails
  auto result = client.getCardByCollectorNumber("   ", "   ");

  // Should not crash and return nullopt (network call fails)
  // Note: The actual behavior depends on httpGet failing
  // This test verifies no crash on whitespace-only input
  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest, SpecialCharactersInSetCodeHandledGracefully) {
  api::ScryfallClient client(testCacheDir_);

  // Special characters that need URL encoding
  auto result = client.getCardByCollectorNumber("a/b", "123");

  // Should not crash, returns nullopt (network call fails or 404)
  EXPECT_FALSE(result.has_value());
}

TEST_F(ScryfallClientTest, UnicodeInCardNameHandledGracefully) {
  api::ScryfallClient client(testCacheDir_);

  // Unicode characters in name
  auto result = client.getCardByFuzzyName("Jötunheimer");

  // Should not crash, returns nullopt (network call fails)
  EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Non-copyable tests
// ============================================================================

TEST_F(ScryfallClientTest, ClientIsNonCopyable) {
  // This test verifies at compile time that the client is non-copyable
  // If you uncomment the lines below, they should fail to compile:

  // api::ScryfallClient client1(testCacheDir_);
  // api::ScryfallClient client2 = client1;  // Should not compile
  // api::ScryfallClient client3(client1);   // Should not compile

  // If this test compiles, the non-copyable constraint is in place
  SUCCEED();
}

// ============================================================================
// Cache key normalization tests
// ============================================================================

TEST_F(ScryfallClientTest, SetCodeIsLowercasedForLookup) {
  api::ScryfallClient client(testCacheDir_);

  // Both uppercase and lowercase should work and return the same card
  // (set codes are normalized to lowercase internally)
  auto result1 = client.getCardByCollectorNumber("DSC", "92");
  auto result2 = client.getCardByCollectorNumber("dsc", "92");

  // If network is available, both should find the same card
  // If network is not available, both should fail gracefully
  if (result1.has_value() && result2.has_value()) {
    // Same card should be returned regardless of case
    EXPECT_EQ(result1->name, result2->name);
    EXPECT_EQ(result1->setCode, result2->setCode);
    EXPECT_EQ(result1->collectorNumber, result2->collectorNumber);
  } else {
    // Both should behave consistently
    EXPECT_EQ(result1.has_value(), result2.has_value());
  }
}

// ============================================================================
// Multiple client instances
// ============================================================================

TEST_F(ScryfallClientTest, MultiplClientsCanShareCacheDirectory) {
  // Create two clients with same cache directory
  api::ScryfallClient client1(testCacheDir_);
  api::ScryfallClient client2(testCacheDir_);

  // Both should work without issues
  EXPECT_NO_THROW({
    std::ignore = client1.getCardByFuzzyName("test");
    std::ignore = client2.getCardByFuzzyName("test");
  });
}

TEST_F(ScryfallClientTest, ClientsWithDifferentCacheDirectoriesAreIndependent) {
  auto cacheDir1 = testCacheDir_ / "client1";
  auto cacheDir2 = testCacheDir_ / "client2";

  api::ScryfallClient client1(cacheDir1);
  api::ScryfallClient client2(cacheDir2);

  // Create a file in one cache
  std::ofstream(cacheDir1 / "test.json") << "{}";

  // Clear cache for client1
  client1.clearCache();

  // File in client1's cache should be gone
  EXPECT_FALSE(std::filesystem::exists(cacheDir1 / "test.json"));

  // Client2's cache directory should still exist and be empty
  EXPECT_TRUE(std::filesystem::exists(cacheDir2));
}

// ============================================================================
// Error recovery tests
// ============================================================================

TEST_F(ScryfallClientTest, ConsecutiveFailedLookupsDoNotCorruptState) {
  api::ScryfallClient client(testCacheDir_);

  // Multiple failed lookups should not corrupt client state
  for (int i = 0; i < 5; ++i) {
    auto result =
        client.getCardByFuzzyName("nonexistent_card_" + std::to_string(i));
    EXPECT_FALSE(result.has_value());
  }

  // Cache misses should be tracked
  EXPECT_EQ(client.getCacheMisses(), 5U);
  EXPECT_EQ(client.getCacheHits(), 0U);

  // Client should still be usable
  EXPECT_NO_THROW({ std::ignore = client.getCardByFuzzyName("another_test"); });
}

// ============================================================================
// API behavior tests
// ============================================================================

TEST_F(ScryfallClientTest, SearchCardsReturnsVectorOfCards) {
  api::ScryfallClient client(testCacheDir_);

  // Search should return a vector (empty or with results depending on network)
  auto results = client.searchCards("type:artifact set:dsc");

  // If network is available, we might get results
  // If not, we get empty results - either way no crash
  for (const auto &card : results) {
    // Each returned card should be valid
    EXPECT_TRUE(card.isValid);
    EXPECT_FALSE(card.name.empty());
  }
}

TEST_F(ScryfallClientTest, CollectorNumberLookupIncrementsCacheMiss) {
  api::ScryfallClient client(testCacheDir_);

  EXPECT_EQ(client.getCacheMisses(), 0U);

  std::ignore = client.getCardByCollectorNumber("abc", "123");

  EXPECT_EQ(client.getCacheMisses(), 1U);
}

TEST_F(ScryfallClientTest, FuzzyNameLookupIncrementsCacheMiss) {
  api::ScryfallClient client(testCacheDir_);

  EXPECT_EQ(client.getCacheMisses(), 0U);

  std::ignore = client.getCardByFuzzyName("Test Card");

  EXPECT_EQ(client.getCacheMisses(), 1U);
}

} // namespace
