if(TARGET Vulkan::Vulkan)
    return()
endif()

message(STATUS "Third-party (external) targets available: 'Vulkan::Vulkan', 'Vulkan::glslc', 'Vulkan::Headers' and 'Vulkan::glslangValidator'")

find_package(Vulkan REQUIRED)
