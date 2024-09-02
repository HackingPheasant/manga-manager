if(TARGET glm::glm)
    return()
endif()

include(FeatureSummary)

message(VERBOSE "Third-party targets available: 'glm::glm'")

find_package(glm)
# TODO Put a version check here as glm 1.0.0 defines the target
# but for now this is good enough
if(NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif()

set_package_properties(glm PROPERTIES
    URL "https://glm.g-truc.net/"
    DESCRIPTION "C++ mathematics library for graphics software"
    TYPE REQUIRED)
