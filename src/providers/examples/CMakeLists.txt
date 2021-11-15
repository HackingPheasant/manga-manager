include(fmt)
include(nlohmann_json)

add_executable("mangadex-test" main.cpp)

target_link_libraries("mangadex-test" PUBLIC
    project::options
    manga-manager::core
    manga-manager::providers
    fmt::fmt
    nlohmann_json::nlohmann_json
    )
