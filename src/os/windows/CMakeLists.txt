# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

message(FATAL_ERROR "Currently an unimplemented platform!")

add_library(manga-manager.os.windows)
add_library(manga-manager::os::windows ALIAS manga-manager.os.windows)

target_compile_definitions(manga-manager.os.windows PUBLIC VK_USE_PLATFORM_WIN32_KHR WSI_WIN32)

target_link_libraries(manga-manager.os.windows PUBLIC
    project::options
)
