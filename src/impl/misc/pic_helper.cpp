
#include <misc/pic_helper.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace cs {

void displayResults(cv::Mat pic) {
  cv::imshow("Card", pic);
  cv::waitKey(0);
}

bool saveImage(const std::filesystem::path &savePath, cv::Mat pic, std::string name) {
    try {
        // Ensure the directory exists
        std::filesystem::create_directories(savePath);

        // Generate a unique name if not provided
        if (name.empty()) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::ostringstream oss;
            oss << "image_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S")
                << "_" << std::setfill('0') << std::setw(3) << ms.count() << ".png";
            name = oss.str();
        }

        // Construct the full path
        auto fullPath = savePath / name;

        // Save the image
        if (!cv::imwrite(fullPath.string(), pic)) {
            throw std::runtime_error("Failed to write image to file");
        }

        spdlog::info("Saved image to {}",fullPath.string());
        return true;
    } catch (const std::exception &e) {
        spdlog::critical("Error saving image: {}",e.what());
        return false;
    }
}
} // namespace cs