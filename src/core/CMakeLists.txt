# 1. define
add_library("manga-manager_core")
add_library("manga-manager::core" ALIAS "manga-manager_core")

message(STATUS "manga-manager: creating target 'manga-manager::core'")

include(GNUInstallDirs)
target_include_directories("manga-manager_core" PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# 2. target sources
target_sources("manga-manager_core"
    PRIVATE
    src/http.cpp
    PUBLIC
    include/manga-manager/http.h
    )

# 3. dependencies
include(cpr)
include(fmt)

target_link_libraries("manga-manager_core" PUBLIC
    project::options
    project::warnings
    cpr::cpr
    fmt::fmt
    )

# 4. installation

# 5. tests and examples
