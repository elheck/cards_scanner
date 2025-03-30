#include <workflow/detection_builder.hpp>
#include <detection/card_detector.hpp>
#include <detection/region_extraction.hpp>
#include <detection/tilt_corrector.hpp>
#include <stdexcept>

namespace workflow {

DetectionBuilder::DetectionBuilder(CardType type) : type_(type) {}

cv::Mat DetectionBuilder::process(const std::filesystem::path& imagePath) {
    switch (type_) {
        case CardType::modernNormal:
            return processModernNormal(imagePath);
        default:
            throw std::runtime_error("Unsupported card type");
    }
}

cv::Mat DetectionBuilder::processModernNormal(const std::filesystem::path& imagePath) {
    // Process the card using the detection pipeline
    auto card = detect::processCards(imagePath);

    // Apply tilt correction
    card = detect::correctCardTilt(card);

    // Extract bounding boxes
    auto name_box = detect::extractNameRegion(card);
    auto collector_box = detect::extractCollectorNumberRegionModern(card);
    auto set_name_box = detect::extractSetNameRegionModern(card);
    auto art_box = detect::extractArtRegionRegular(card);

    // Draw all bounding boxes on the card with different colors
    cv::Mat result = card.clone();
    
    // Name region - Green
    cv::rectangle(result, name_box, cv::Scalar(0, 255, 0), 2);
    
    // Collector number region - Red
    cv::rectangle(result, collector_box, cv::Scalar(0, 0, 255), 2);
    
    // Set name region - Blue
    cv::rectangle(result, set_name_box, cv::Scalar(255, 0, 0), 2);
    
    // Art region - Yellow
    cv::rectangle(result, art_box, cv::Scalar(0, 255, 255), 2);

    return result;
}
} // namespace workflow