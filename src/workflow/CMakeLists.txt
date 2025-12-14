add_library(workflow_lib
    impl/detection_builder.cpp
)

target_include_directories(workflow_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(workflow_lib
    PUBLIC
        card_processor_lib
    PRIVATE
        ${OpenCV_LIBS}
        libassert::assert
)
