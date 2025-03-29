#pragma once
#include <opencv2/opencv.hpp>



namespace detect
{

    class RegionExtractor
    {
        public:
        explicit RegionExtractor(cv::Mat &image);
        ~RegionExtractor();

        [[nodiscard]] cv::Mat extractNameRegion();
        [[nodiscard]] cv::Mat extractCollectorNumberRegion();
        [[nodiscard]] cv::Mat extarctSetNameRegion();
        [[nodiscard]] cv::Mat extractArtRegion();
        [[nodiscard]] cv::Mat extractTextRegion();


        private:

        cv::Mat image_;
    };

} // namespace detect
