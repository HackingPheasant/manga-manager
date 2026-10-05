# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

message(FATAL_ERROR "Currently an unimplemented platform!")

add_library(manga-manager.os.apple)
add_library(manga-manager::os::apple ALIAS manga-manager.os.apple)

target_compile_definitions(manga-manager.os.apple PUBLIC VK_USE_PLATFORM_METAL_EXT WSI_APPLE)

target_link_libraries(manga-manager.os.apple PUBLIC
    project::options
)
