add_library("manga-manager_core")
add_library("manga-manager::core" ALIAS "manga-manager_core")

message(VERBOSE "manga-manager: creating target 'manga-manager::core'")

include(fmt)
include(nlohmann_json)

target_sources("manga-manager_core"
    PRIVATE
    http.cpp
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    FILES
    http.h
)

target_link_libraries("manga-manager_core" PUBLIC
    project::options
    fmt::fmt
    nlohmann_json::nlohmann_json
)
