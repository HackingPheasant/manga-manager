if(TARGET fmt::fmt)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'fmt::fmt'")

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 7.1.3
    )

FetchContent_MakeAvailable(fmt)
