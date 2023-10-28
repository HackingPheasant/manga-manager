if(TARGET Vulkan::Vulkan)
    return()
endif()

message(VERBOSE "Third-party targets available: 'Vulkan::Vulkan', 'Vulkan::glslc', 'Vulkan::Headers' and 'Vulkan::glslangValidator'")

find_package(Vulkan REQUIRED)
