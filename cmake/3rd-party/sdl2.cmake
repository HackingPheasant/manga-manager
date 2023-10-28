if(TARGET SDL2::SDL2)
    return()
endif()

message(VERBOSE "Third-party targets available: 'SDL2::SDL2'")

find_package(SDL2 CONFIG REQUIRED)
