# SPDX-FileCopyrightText: © 2024 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

include(InstallRequiredSystemLibraries)

# We will only set info that doesn't already defualt to info supplied elsewhere.
set(CPACK_PACKAGE_VENDOR "HackingPheasant")
set(CPACK_PACKAGE_DESCRIPTION "Manga manager, built with C++. Initially (and still is) a project primarily intended for learning.")
#set(CPACK_PACKAGE_ICON "")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES
    \\.git/
    build/
    out/
    Testing/
    ".*~$"
)

include(CPack)
