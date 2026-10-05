# SPDX-FileCopyrightText: © 2021 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.ui)
add_library(manga-manager::ui ALIAS manga-manager.ui)

include(CompileShaders)

target_compile_shaders(manga-manager.ui
    PRIVATE assets/vulkantut.vert assets/vulkantut.frag
    COMPILE_OPTIONS --target-env vulkan1.3 -x)

target_sources(manga-manager.ui
    PRIVATE
      vulkan_renderer.cpp
    
    PUBLIC
    FILE_SET HEADERS
    FILES
      vulkan_renderer.hpp
)

target_link_libraries(manga-manager.ui PUBLIC
    project::options
    manga-manager::os
    Vulkan::Headers
    glm::glm
)
