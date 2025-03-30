# Find the Conan-installed packages
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)
find_package(libassert REQUIRED)

# Define the core library
add_library(card_processor_lib
    impl/detection/card_detector.cpp
    impl/detection/tilt_corrector.cpp
    impl/detection/region_extraction.cpp
    impl/misc/pic_helper.cpp
    impl/misc/path_helper.cpp
)

# Include directories for the core library
target_include_directories(card_processor_lib 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/include
    PRIVATE
        ${OpenCV_INCLUDE_DIRS} 
)

# Link libraries for the core library
target_link_libraries(card_processor_lib 
    PRIVATE
        ${OpenCV_LIBS}
        spdlog::spdlog
        libassert::assert
)

# Add workflow library
add_library(workflow_lib
    impl/workflow/detection_builder.cpp
)

target_include_directories(workflow_lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/include
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(workflow_lib
    PUBLIC
        card_processor_lib
    PRIVATE
        ${OpenCV_LIBS}
)

# Add executable
add_executable(card_scanner
    main.cpp
)

target_include_directories(card_scanner
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}    
)

# Link the executable with both libraries
target_link_libraries(card_scanner 
    PRIVATE
        card_processor_lib
        workflow_lib
        spdlog::spdlog
        libassert::assert
)