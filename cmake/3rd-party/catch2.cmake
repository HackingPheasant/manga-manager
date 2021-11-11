if(TARGET Catch2::Catch2)
    return()
endif()

message(STATUS "Third-party (external) targets available:'Catch2::Catch2'")

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 2dc5a5f
    )

FetchContent_MakeAvailable(Catch2)
