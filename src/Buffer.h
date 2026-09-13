#pragma once

#include "VkCommon.h"

// RAII pair of VkBuffer + its VkDeviceMemory.
//
// Takes the raw handles rather than a Device wrapper: Qt's QVulkanWindow owns
// the device now, and hands out VkDevice / VkPhysicalDevice directly.
//
// One vkAllocateMemory per buffer is fine at this scale but is the wrong
// pattern for a real engine, where allocations are sub-allocated from large
// blocks (see VulkanMemoryAllocator). This is the seam for that change.
class Buffer {
public:
    Buffer() = default;
    Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    VkBuffer handle() const { return buffer_; }
    VkDeviceSize size() const { return size_; }

    void* map();
    void unmap();

    // Releases the handles early; safe to call more than once. Needed because
    // Qt tears the device down in releaseResources(), before this object's
    // destructor would otherwise run.
    void destroy();

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
};
