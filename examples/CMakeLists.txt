# SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

add_executable(provider-example provider-example.cpp)
add_executable(ui-example ui-example.cpp)

target_link_libraries(provider-example PUBLIC
    project::options
    manga-manager::core
    manga-manager::providers
    nlohmann_json::nlohmann_json
)

target_link_libraries(ui-example PUBLIC
    project::options
    manga-manager::ui
)

install(TARGETS
    provider-example
    ui-example
    DESTINATION examples)
