# Find the Conan-installed packages
find_package(OpenCV REQUIRED)
find_package(Tesseract REQUIRED)
find_package(spdlog REQUIRED)
find_package(libassert REQUIRED)
find_package(Microsoft.GSL REQUIRED)

# Add the library subdirectories
add_subdirectory(misc)
add_subdirectory(detection)
add_subdirectory(workflow)

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