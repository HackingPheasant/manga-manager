# 1. define
add_library("manga-manager_providers")
add_library("manga-manager::providers" ALIAS "manga-manager_providers")

message(STATUS "manga-manager: creating target 'manga-manager::provides'")

include(GNUInstallDirs)
target_include_directories("manga-manager_providers" PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# 2. target sources
target_sources("manga-manager_providers"
    PRIVATE
    src/mangadex.cpp
    PUBLIC
    include/manga-manager/providers/mangadex.h
    )

# 3. dependencies
include(fmt)
include(nlohmann_json)

target_link_libraries("manga-manager_providers" PUBLIC
    project::options
    project::warnings
    manga-manager::core
    fmt::fmt
    nlohmann_json::nlohmann_json
    )

# 4. installation

# 5. tests and examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
