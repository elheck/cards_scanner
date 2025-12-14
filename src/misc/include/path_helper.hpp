#pragma once

#include <filesystem>

namespace misc {
    [[nodiscard]] std::filesystem::path getSamplesPath();
    [[nodiscard]] std::filesystem::path getTestSamplesPath();
} // namespace misc