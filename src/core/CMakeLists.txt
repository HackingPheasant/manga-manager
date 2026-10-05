# SPDX-FileCopyrightText: © 2020 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_library(manga-manager.core)
add_library(manga-manager::core ALIAS manga-manager.core)

target_sources(manga-manager.core
  PRIVATE
    http.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      http.hpp
)

target_link_libraries(manga-manager.core PUBLIC
    project::options
    #nlohmann_json::nlohmann_json
)
