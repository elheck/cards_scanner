#pragma once

#include <detection/card_detector.hpp>
#include <detection/region_extraction.hpp>
#include <detection/tilt_corrector.hpp>
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace workflow {

enum class CardType {
    modernNormal,
    // Can be extended with more card types in the future
};

class DetectionBuilder {
public:
    explicit DetectionBuilder(CardType type);
    
    // Build and process the card image
    cv::Mat process(const std::filesystem::path& imagePath);

private:
    CardType type_;
    cv::Mat processModernNormal(const std::filesystem::path& imagePath);
};
}