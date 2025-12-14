#pragma once

#include <opencv2/opencv.hpp>

#include <filesystem>

namespace workflow {

enum class CardType {
    modernNormal,
    // Can be extended with more card types in the future
};

class DetectionWorkflow {
public:
    explicit DetectionWorkflow(CardType type);
    
    // Build and process the card image
    cv::Mat process(const std::filesystem::path& imagePath);

private:
    CardType type_;
    cv::Mat nameImage_;
    cv::Mat collectorNumberImage_;
    cv::Mat setNameImage_;
    cv::Mat artImage_;
    cv::Mat processModernNormal(const std::filesystem::path& imagePath);
    void readTextFromRegions();
};
}