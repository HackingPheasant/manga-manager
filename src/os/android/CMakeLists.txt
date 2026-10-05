# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

message(FATAL_ERROR "Currently an unimplemented platform!")

add_library(manga-manager.os.android)
add_library(manga-manager::os::android ALIAS manga-manager.os.android)

target_compile_definitions(manga-manager.os.android PUBLIC VK_USE_PLATFORM_ANDROID_KHR WSI_ANDROID)

target_link_libraries(manga-manager.os.android PUBLIC
    project::options
)
