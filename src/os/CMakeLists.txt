add_library("manga-manager_os")
add_library("manga-manager::os" ALIAS "manga-manager_os")

message(VERBOSE "manga-manager: creating target 'manga-manager::os'")

target_sources("manga-manager_os"
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    FILES
    window.h
)

target_link_libraries("manga-manager_os" PUBLIC
    project::options
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(windows)
    target_link_libraries("manga-manager_os" PUBLIC
        os::windows
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_subdirectory(android)
    target_link_libraries("manga-manager_os" PUBLIC
        os::android
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(apple)
    target_link_libraries("manga-manager_os" PUBLIC
        os::apple
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(linux)
    target_link_libraries("manga-manager_os" PUBLIC
        os::linux
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Fuchsia")
    add_subdirectory(fuchsia)
    target_link_libraries("manga-manager_os" PUBLIC
        os::fuchsia
    )
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()
