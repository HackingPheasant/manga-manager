# SPDX-FileCopyrightText: © 2025 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
target_compile_options(${project_name} INTERFACE --coverage)
target_link_libraries(${project_name} INTERFACE --coverage)
endif()
