# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

message(FATAL_ERROR "Currently an unimplemented platform!")

add_library(manga-manager.os.fuchsia)
add_library(manga-manager::os::fuchsia ALIAS manga-manager.os.fuchsia)

target_compile_definitions(manga-manager.os.fuchsia PUBLIC VK_USE_PLATFORM_FUCHSIA WSI_FUCHSIA)

target_link_libraries(manga-manager.os.fuchsia PUBLIC
    project::options
)
