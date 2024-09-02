if(TARGET Vulkan::Headers)
    return()
endif()

include(FeatureSummary)

message(VERBOSE "Third-party targets available: 'Vulkan::Headers'")

# We require Vulkan version ≥ 1.3.265, earliest version when
# vulkan_hpp_macros.hpp was available.
# VulkanHeaders cmake files started shipping with SDK versions 1.3.275 onwards
#
# So we can safely drop FindVulkan.cmake which is soon to be deprecated.
# https://gitlab.kitware.com/cmake/cmake/-/issues/25617
find_package(VulkanHeaders 1.3.275 REQUIRED CONFIG HINTS $ENV{VULKAN_SDK})

set_package_properties(VulkanHeaders PROPERTIES
    URL "https://github.com/KhronosGroup/Vulkan-Headers"
    DESCRIPTION "Provides Vulkan function prototypes to interact with the graphics API"
    TYPE REQUIRED)
