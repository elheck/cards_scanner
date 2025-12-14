add_library(api_lib STATIC
    impl/scryfall_client.cpp
)

target_include_directories(api_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(api_lib PUBLIC
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)
