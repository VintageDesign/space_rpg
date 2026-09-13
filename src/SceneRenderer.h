#pragma once

// Must precede any Qt Vulkan header (QVulkanWindowRenderer, QVulkanWindow):
// those #define VK_NO_PROTOTYPES before including vulkan.h, which would
// otherwise suppress the vk* declarations this file and its callers use.
#include <vulkan/vulkan.h>

#include <QElapsedTimer>
#include <QVulkanWindowRenderer>

#include <memory>
#include <vector>

#include "Buffer.h"
#include "Pipeline.h"
#include "engine/RenderQueue.h"

class Controller;
class QVulkanWindow;

namespace engine {
class SceneTree;
}

// Drives the game loop from Qt's frame callback: ticks the scene tree, then
// draws its render queue as a single batched, z-sorted triangle list.
class SceneRenderer : public QVulkanWindowRenderer {
public:
    SceneRenderer(QVulkanWindow* window, engine::SceneTree* tree,
                  Controller* controller);

    void initResources() override;
    void releaseResources() override;
    void startNextFrame() override;

private:
    void uploadVertices(int frame);

    QVulkanWindow* window_ = nullptr;
    engine::SceneTree* tree_ = nullptr;
    Controller* controller_ = nullptr;

    std::unique_ptr<Pipeline> pipeline_;
    // One per in-flight frame, so a frame never rewrites a buffer the GPU is
    // still reading for an earlier one.
    std::vector<Buffer> vertexBuffers_;

    engine::RenderQueue queue_;
    std::vector<engine::Vertex2D> vertices_;
    QElapsedTimer frameTimer_;
};
