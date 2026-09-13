#pragma once

#include "VkCommon.h"
#include "engine/RenderQueue.h"

#include <array>
#include <cstddef>

// Vulkan vertex input description for engine::Vertex2D.
namespace VertexLayout {

inline VkVertexInputBindingDescription binding() {
    VkVertexInputBindingDescription desc{};
    desc.binding = 0;
    desc.stride = sizeof(engine::Vertex2D);
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

inline std::array<VkVertexInputAttributeDescription, 2> attributes() {
    std::array<VkVertexInputAttributeDescription, 2> attrs{};

    attrs[0].location = 0;
    attrs[0].binding = 0;
    attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[0].offset = offsetof(engine::Vertex2D, pos);

    attrs[1].location = 1;
    attrs[1].binding = 0;
    attrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attrs[1].offset = offsetof(engine::Vertex2D, color);

    return attrs;
}

}  // namespace VertexLayout
