#include <pic_helper.hpp>
#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <string>
#include <system_error>

namespace misc {

namespace {
    constexpr int milliseconds_mod = 1000;  // For timestamp generation
}

void displayResults(const cv::Mat& pic) {
    cv::imshow("Result", pic);
    cv::waitKey(0);
}

bool saveImage(const std::filesystem::path &savePath, const cv::Mat& pic, std::string name) {
    try {
        // Create directories if they don't exist
        std::error_code ec;
        if (!std::filesystem::exists(savePath)) {
            std::filesystem::create_directories(savePath, ec);
            ASSERT(!ec, "Failed to create directory: {}", ec.message());
        }

        // If no name is provided, generate one with timestamp
        if (name.empty()) {
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % milliseconds_mod;
            
            name = "card_" + std::to_string(ms.count()) + ".jpg";
        }

        // Ensure name ends with .jpg
        if (name.find(".jpg") == std::string::npos) {
            name += ".jpg";
        }

        auto full_path = savePath / name;

        // Save the image
        if (!cv::imwrite(full_path.string(), pic)) {
            throw std::runtime_error("Failed to write image to file");
        }

        spdlog::info("Saved image to {}", full_path.string());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error saving image: {}", e.what());
        return false;
    }
}

void checkImage(const cv::Mat &pic, const std::string &operationName) {
    if (pic.empty()) {
        spdlog::critical("Error in {}: Image is empty", operationName);
        throw std::runtime_error("Image is empty");
    }
    if (pic.channels() != 3) {
        throw std::runtime_error("Image must have 3 channels (RGB)");
    }
}

} // namespace misc