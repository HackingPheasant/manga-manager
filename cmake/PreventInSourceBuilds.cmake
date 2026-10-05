# SPDX-FileCopyrightText: © 2020 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

# This function will prevent in-source builds
function(PreventInSourceBuilds)
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR "In-source builds are not allowed!"
        "Please create a separate build directory and run cmake from there."
        "This process created the file `CMakeCache.txt' and the directory "
        "`CMakeFiles'. Please delete them.")
    endif()
endfunction()

preventinsourcebuilds()
