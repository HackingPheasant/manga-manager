# SPDX-FileCopyrightText: © 2021 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.providers)
add_library(manga-manager::providers ALIAS manga-manager.providers)

target_sources(manga-manager.providers
  PRIVATE
    mangadex.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      mangadex.hpp
)

target_link_libraries(manga-manager.providers PUBLIC
    project::options
    manga-manager::core
)
