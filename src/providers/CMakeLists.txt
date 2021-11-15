add_library("manga-manager_providers")
add_library("manga-manager::providers" ALIAS "manga-manager_providers")

message(STATUS "manga-manager: creating target 'manga-manager::provides'")

include(fmt)

target_sources("manga-manager_providers"
    PRIVATE
    mangadex.cpp
    mangadex.h
    )

target_link_libraries("manga-manager_providers" PUBLIC
    project::options
    manga-manager::core
    fmt::fmt
    )

target_include_directories("manga-manager_providers" PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
