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
    
    // Extract regions in order
    auto card_with_name = detect::extractNameRegion(card);
    
    return card_with_name;
}
} // namespace workflow