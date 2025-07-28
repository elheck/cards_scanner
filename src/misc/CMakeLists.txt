add_library(misc_lib
    impl/pic_helper.cpp
    impl/path_helper.cpp
)

target_include_directories(misc_lib 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
