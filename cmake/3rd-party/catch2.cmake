if(TARGET Catch2::Catch2)
    return()
endif()

message(VERBOSE "Third-party targets available:'Catch2::Catch2'")

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
    FIND_PACKAGE_ARGS
    )

FetchContent_MakeAvailable(Catch2)
