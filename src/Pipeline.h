#pragma once

#include "VkCommon.h"

#include <string>

// Matches the push_constant block in shaders/sprite.vert.
struct PushConstants {
    float axisX[2];
    float axisY[2];
    float origin[2];
};

// Owns the graphics pipeline and its layout.
//
// The render pass and framebuffers are no longer ours: QVulkanWindow creates
// them, so the pipeline is built against window->defaultRenderPass() and must
// match that pass's sample count.
class Pipeline {
public:
    Pipeline(VkDevice device, VkRenderPass renderPass,
             VkSampleCountFlagBits samples);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    VkPipeline handle() const { return pipeline_; }
    VkPipelineLayout layout() const { return layout_; }

private:
    VkShaderModule createShaderModule(const std::string& path) const;

    VkDevice device_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
};
