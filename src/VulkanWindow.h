#pragma once

// Must precede QVulkanWindow: it #defines VK_NO_PROTOTYPES before including
// vulkan.h, which would otherwise suppress the vk* declarations used
// elsewhere in this translation unit.
#include <vulkan/vulkan.h>

#include <QVulkanWindow>

class Controller;

namespace engine {
class SceneTree;
}

// The QWindow that receives input over the Vulkan view; forwards keys to the
// scene tree and creates the renderer that ticks it.
class VulkanWindow : public QVulkanWindow {
    Q_OBJECT

public:
    VulkanWindow(engine::SceneTree* tree, Controller* controller);

    QVulkanWindowRenderer* createRenderer() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    engine::SceneTree* tree_ = nullptr;
    Controller* controller_ = nullptr;
};
