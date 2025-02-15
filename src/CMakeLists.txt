

target_include_directories(ocr_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

# Dependency handling
find_package(OpenCV REQUIRED)
find_package(Tesseract REQUIRED)

add_executable(mtg_ocr main.cpp)

target_link_libraries(mtg_ocr PRIVATE 
    ${OpenCV_LIBS} 
    ${Tesseract_LIBRARIES}
)


# Compiler options
target_compile_options(ocr_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
)

