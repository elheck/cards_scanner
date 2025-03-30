# Find the Conan-installed packages
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)
find_package(libassert REQUIRED)
find_package(Microsoft.GSL REQUIRED)

# Define the misc utility library
add_library(misc_lib
    impl/misc/pic_helper.cpp
    impl/misc/path_helper.cpp
)

target_include_directories(misc_lib 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/include
    PRIVATE
        ${OpenCV_INCLUDE_DIRS} 
)

target_link_libraries(misc_lib 
    PRIVATE
        ${OpenCV_LIBS}
        spdlog::spdlog
        libassert::assert
        Microsoft.GSL::GSL
)

# Define the core library
add_library(card_processor_lib
    impl/detection/card_detector.cpp
    impl/detection/tilt_corrector.cpp
    impl/detection/region_extraction.cpp
)

target_include_directories(card_processor_lib 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/include
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

# Link the executable with workflow_lib (which transitively includes card_processor_lib and misc_lib)
target_link_libraries(card_scanner 
    PRIVATE
        workflow_lib
        misc_lib
        spdlog::spdlog
        libassert::assert
        Microsoft.GSL::GSL
)