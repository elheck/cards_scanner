#include <misc/path_helper.hpp>

namespace misc {

[[nodiscard]] std::filesystem::path getSamplesPath() {
    return std::filesystem::path(SAMPLE_DATA_FOLDER);
}

} // namespace misc