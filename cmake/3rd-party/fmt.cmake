if(TARGET fmt::fmt)
    return()
endif()

message(VERBOSE "Third-party targets available: 'fmt::fmt'")

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
    FIND_PACKAGE_ARGS
    )

FetchContent_MakeAvailable(fmt)
