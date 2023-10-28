add_library("manga-manager_providers")
add_library("manga-manager::providers" ALIAS "manga-manager_providers")

message(VERBOSE "manga-manager: creating target 'manga-manager::providers'")

include(fmt)

target_sources("manga-manager_providers"
    PRIVATE
    mangadex.cpp
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    FILES
    mangadex.h
    )

target_link_libraries("manga-manager_providers" PUBLIC
    project::options
    manga-manager::core
    fmt::fmt
    )

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
