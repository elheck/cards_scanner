add_library(card_processor_lib
    impl/card_detector.cpp
    impl/tilt_corrector.cpp
    impl/region_extraction.cpp
    impl/card_text_ocr.cpp
)

target_include_directories(card_processor_lib 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${OpenCV_INCLUDE_DIRS} 
)

target_link_libraries(card_processor_lib 
    PUBLIC
        misc_lib
    PRIVATE
        ${OpenCV_LIBS}
        spdlog::spdlog
        libassert::assert
        Microsoft.GSL::GSL
        Tesseract::libtesseract
       # leptonica
)
