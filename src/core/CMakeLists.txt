add_library("manga-manager_core")
add_library("manga-manager::core" ALIAS "manga-manager_core")

message(STATUS "manga-manager: creating target 'manga-manager::core'")

include(fmt)
include(nlohmann_json)

target_sources("manga-manager_core"
    PRIVATE
    http.cpp
    PUBLIC
    http.h
    )

target_link_libraries("manga-manager_core" PUBLIC
    project::options
    fmt::fmt
    nlohmann_json::nlohmann_json
    )

target_include_directories("manga-manager_core" PUBLIC ${CMAKE_CURRENT_LIST_DIR})
