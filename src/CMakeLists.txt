

# Dependency handling
find_package(OpenCV REQUIRED)

add_executable(mtg_ocr app/main.cpp
    core/imageProcessor.cpp
)

target_include_directories(mtg_ocr PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(mtg_ocr PRIVATE 
    ${OpenCV_LIBS} 
    ${Tesseract_LIBRARIES}
)


# Compiler options
target_compile_options(mtg_ocr PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
)

