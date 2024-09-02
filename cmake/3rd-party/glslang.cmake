if(TARGET glslang::glslang-standalone)
    return()
endif()

include(FeatureSummary)

message(VERBOSE "Third-party targets available: 'glslang::glslang (and related), glslang::glslang-standalone, glslang::SPIRV (and related) etc.'")

find_package(glslang REQUIRED)

set_package_properties(glslang PROPERTIES
    URL "https://github.com/KhronosGroup/glslang"
    DESCRIPTION "Offical reference compiler for OpenGL shading languages"
    TYPE REQUIRED)
