#include <filesystem>
#include <misc/pic_helper.hpp>
#include <stdexcept>
namespace cs {

void displayResults(cv::Mat pic) {
  cv::imshow("Card", pic);
  cv::waitKey(0);
}

bool saveResults(const std::filesystem::path &originalPath, cv::Mat pic) {

  // 1. Get the parent directory of the original image
  auto parentDir = originalPath.parent_path();

  // 2. Generate a subfolder name based on the current time up to the minute
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M");
  auto subfolder = parentDir / oss.str();

  // 3. Create the subfolder
  try {
    std::filesystem::create_directory(subfolder);
  } catch (std::exception &e) {
    std::cerr << "Failed to create directory: " << e.what() << std::endl;
    return false;
  }
  auto nowCard = std::chrono::system_clock::now();
  auto in_time_t_card = std::chrono::system_clock::to_time_t(nowCard);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                nowCard.time_since_epoch()) %
            1000;

  std::ostringstream fileName;
  fileName << "card_" << std::put_time(std::localtime(&in_time_t_card), "%S")
           << "_" << ms.count() << ".png";

  auto outPath = subfolder / fileName.str();
  if (!cv::imwrite(outPath.string(), pic)) {
      throw std::runtime_error("Can't write pic to file");
  }

  std::cout << "Saved card(s) to " << subfolder << std::endl;
  return true;
}
} // namespace cs