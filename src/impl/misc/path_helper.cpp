#include <misc/path_helper.hpp>

namespace misc {

[[nodiscard]] std::filesystem::path getSamplesPath() {
    return std::filesystem::path(SAMPLE_DATA_FOLDER);
}

[[nodiscard]] std::filesystem::path getTestSamplesPath() {
    return std::filesystem::path(TEST_SAMPLES_FOLDER);
}

} // namespace misc