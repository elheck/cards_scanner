
# Find the Conan-installed packages
find_package(OpenCV REQUIRED)


# Define the core library
add_library(card_processor_lib
    impl/detection/card_detector.cpp
    impl/detection/tilt_corrector.cpp
    impl/misc/pic_helper.cpp
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
)

# Add executable
add_executable(card_scanner
    main.cpp
)

target_include_directories(card_scanner
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}    
)


# Link the executable with the core library
target_link_libraries(card_scanner 
    PRIVATE
        card_processor_lib
)