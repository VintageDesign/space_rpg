#include "SceneRenderer.h"

#include <QCoreApplication>
#include <QEvent>
#include <QVulkanWindow>

#include <algorithm>
#include <cstring>

#include "Controller.h"
#include "engine/SceneTree.h"

namespace {

constexpr VkDeviceSize kInitialVertexCapacity = 4096;
constexpr float kMaxFrameSeconds = 0.1f;

}  // namespace

SceneRenderer::SceneRenderer(QVulkanWindow* window, engine::SceneTree* tree,
                             Controller* controller)
    : window_(window), tree_(tree), controller_(controller) {}

void SceneRenderer::initResources() {
    // Build against Qt's render pass, matching its sample count.
    pipeline_ = std::make_unique<Pipeline>(window_->device(),
                                           window_->defaultRenderPass(),
                                           window_->sampleCountFlagBits());
    vertexBuffers_.resize(
        static_cast<size_t>(window_->concurrentFrameCount()));
    frameTimer_.start();
}

void SceneRenderer::releaseResources() {
    // Qt destroys the device after this returns, so release explicitly rather
    // than waiting for member destruction.
    vertexBuffers_.clear();
    pipeline_.reset();
}

void SceneRenderer::uploadVertices(int frame) {
    const VkDeviceSize needed = sizeof(engine::Vertex2D) * vertices_.size();
    Buffer& buffer = vertexBuffers_[static_cast<size_t>(frame)];

    if (buffer.size() < needed) {
        VkDeviceSize capacity = std::max(
            buffer.size(), kInitialVertexCapacity * sizeof(engine::Vertex2D));
        while (capacity < needed) {
            capacity *= 2;
        }
        // Safe to replace: Qt has waited for this frame slot's previous use.
        buffer = Buffer(window_->device(), window_->physicalDevice(), capacity,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void* mapped = buffer.map();
    std::memcpy(mapped, vertices_.data(), static_cast<size_t>(needed));
    buffer.unmap();
}

void SceneRenderer::startNextFrame() {
    VkCommandBuffer cb = window_->currentCommandBuffer();
    const QSize imageSize = window_->swapChainImageSize();

    const float dt = std::min(
        static_cast<float>(frameTimer_.nsecsElapsed()) / 1e9f,
        kMaxFrameSeconds);
    frameTimer_.restart();

    tree_->viewportSize = {static_cast<float>(imageSize.width()),
                           static_cast<float>(imageSize.height())};
    tree_->tick(dt);

    queue_.clear();
    tree_->buildRenderQueue(queue_);
    queue_.flatten(vertices_);

    const int frame = window_->currentFrame();
    if (!vertices_.empty()) {
        uploadVertices(frame);
    }

    const QColor color = controller_->clearColor();
    VkClearColorValue clearColor{};
    clearColor.float32[0] = static_cast<float>(color.redF());
    clearColor.float32[1] = static_cast<float>(color.greenF());
    clearColor.float32[2] = static_cast<float>(color.blueF());
    clearColor.float32[3] = 1.0f;

    VkClearDepthStencilValue clearDepthStencil{};
    clearDepthStencil.depth = 1.0f;
    clearDepthStencil.stencil = 0;

    // Qt's default render pass has a depth/stencil attachment, and a third
    // resolve attachment when MSAA is enabled. The count must match.
    VkClearValue clearValues[3]{};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDepthStencil;
    clearValues[2].color = clearColor;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = window_->defaultRenderPass();
    renderPassInfo.framebuffer = window_->currentFramebuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.width =
        static_cast<uint32_t>(imageSize.width());
    renderPassInfo.renderArea.extent.height =
        static_cast<uint32_t>(imageSize.height());
    renderPassInfo.clearValueCount =
        window_->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3u : 2u;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (!vertices_.empty()) {
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline_->handle());

        VkViewport viewport{};
        viewport.width = static_cast<float>(imageSize.width());
        viewport.height = static_cast<float>(imageSize.height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cb, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = renderPassInfo.renderArea.extent;
        vkCmdSetScissor(cb, 0, 1, &scissor);

        // Pixels -> NDC. Vulkan NDC is y-down, matching world space.
        PushConstants pc{};
        pc.scale[0] = 2.0f / viewport.width;
        pc.scale[1] = 2.0f / viewport.height;
        pc.offset[0] = -1.0f;
        pc.offset[1] = -1.0f;
        vkCmdPushConstants(cb, pipeline_->layout(), VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(pc), &pc);

        VkBuffer buffers[] = {
            vertexBuffers_[static_cast<size_t>(frame)].handle()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cb, 0, 1, buffers, offsets);

        vkCmdDraw(cb, static_cast<uint32_t>(vertices_.size()), 1, 0, 0);
    }

    vkCmdEndRenderPass(cb);

    // Exactly once per startNextFrame(). The next frame is requested by posting
    // UpdateRequest directly: requestUpdate() on Wayland waits for a surface
    // frame callback that never arrives for an embedded Vulkan window, which
    // stalls after one frame. Pacing still comes from FIFO (vsync) present.
    window_->frameReady();
    QCoreApplication::postEvent(window_, new QEvent(QEvent::UpdateRequest));
}
