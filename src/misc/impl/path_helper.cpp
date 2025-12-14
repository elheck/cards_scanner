#include <filesystem>
#include <path_helper.hpp>

namespace misc {

std::filesystem::path getSamplesPath() { return {SAMPLE_DATA_FOLDER}; }

std::filesystem::path getTestSamplesPath() { return {TEST_SAMPLES_FOLDER}; }

} // namespace misc