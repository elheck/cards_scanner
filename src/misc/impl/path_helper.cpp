#include <misc/path_helper.hpp>
#include <filesystem>

namespace misc {

std::filesystem::path getSamplesPath() {
    return {SAMPLE_DATA_FOLDER};
}

std::filesystem::path getTestSamplesPath() {
    return {TEST_SAMPLES_FOLDER};
}

} // namespace misc