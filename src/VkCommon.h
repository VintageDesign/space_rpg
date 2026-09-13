#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <stdexcept>
#include <string>

// Wraps any call returning VkResult; throws with the call text and a readable
// result name. Vulkan reports nearly every failure this way, so checking every
// call is the difference between a clear error and a silent blank window.
#define VK_CHECK(call)                                                       \
    do {                                                                     \
        VkResult vk_check_result = (call);                                   \
        if (vk_check_result != VK_SUCCESS) {                                 \
            throw std::runtime_error(std::string(#call) + " failed: " +      \
                                     string_VkResult(vk_check_result));      \
        }                                                                    \
    } while (0)

// Qt offers hostVisibleMemoryIndex() / deviceLocalMemoryIndex(), but those
// return *a* suitable index rather than one guaranteed to satisfy a particular
// buffer's memoryTypeBits. Keep doing the real lookup.
inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                               uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        const bool typeAllowed = typeFilter & (1u << i);
        const bool hasProperties =
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
            properties;
        if (typeAllowed && hasProperties) {
            return i;
        }
    }

    throw std::runtime_error(
        "no memory type satisfies the requested properties");
}
